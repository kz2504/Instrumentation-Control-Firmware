#include "pump_card_protocol.h"

#include <string.h>

static uint16_t pump_card_crc16_update(uint16_t crc, uint8_t byte)
{
  uint8_t bit_index;

  crc ^= (uint16_t)byte << 8;
  for (bit_index = 0U; bit_index < 8U; bit_index++)
  {
    if ((crc & 0x8000U) != 0U)
    {
      crc = (uint16_t)((crc << 1) ^ 0x1021U);
    }
    else
    {
      crc <<= 1;
    }
  }

  return crc;
}

uint16_t pump_card_crc16_ccitt_false(const uint8_t *data, uint16_t len)
{
  uint16_t crc = 0xFFFFU;
  uint16_t index;

  for (index = 0U; index < len; index++)
  {
    crc = pump_card_crc16_update(crc, data[index]);
  }

  return crc;
}

uint16_t pump_card_read_u16_le(const uint8_t *data)
{
  return (uint16_t)data[0] | ((uint16_t)data[1] << 8);
}

uint32_t pump_card_read_u32_le(const uint8_t *data)
{
  return (uint32_t)data[0]
       | ((uint32_t)data[1] << 8)
       | ((uint32_t)data[2] << 16)
       | ((uint32_t)data[3] << 24);
}

void pump_card_write_u16_le(uint8_t *dst, uint16_t value)
{
  dst[0] = (uint8_t)(value & 0xFFU);
  dst[1] = (uint8_t)((value >> 8) & 0xFFU);
}

void pump_card_write_u32_le(uint8_t *dst, uint32_t value)
{
  dst[0] = (uint8_t)(value & 0xFFU);
  dst[1] = (uint8_t)((value >> 8) & 0xFFU);
  dst[2] = (uint8_t)((value >> 16) & 0xFFU);
  dst[3] = (uint8_t)((value >> 24) & 0xFFU);
}

bool pump_card_encode_command_frame(uint8_t command_id,
                                    const uint8_t *payload,
                                    uint16_t payload_len,
                                    uint8_t *frame_out,
                                    uint16_t frame_out_size)
{
  uint16_t crc;

  if ((frame_out == 0) || (frame_out_size < PUMP_CARD_CMD_FRAME_SIZE))
  {
    return false;
  }

  if ((payload_len > PUMP_CARD_CMD_PAYLOAD_MAX) || ((payload_len > 0U) && (payload == 0)))
  {
    return false;
  }

  memset(frame_out, 0, frame_out_size);
  frame_out[0] = PUMP_CARD_PROTO_SOF;
  frame_out[1] = command_id;
  pump_card_write_u16_le(&frame_out[2], payload_len);

  if (payload_len > 0U)
  {
    memcpy(&frame_out[4], payload, payload_len);
  }

  crc = pump_card_crc16_ccitt_false(&frame_out[1], (uint16_t)(3U + payload_len));
  pump_card_write_u16_le(&frame_out[4 + payload_len], crc);
  return true;
}

uint8_t pump_card_decode_command_frame(const uint8_t *frame,
                                       uint16_t frame_len,
                                       PumpCardCommand *command_out)
{
  uint16_t payload_len;
  uint16_t crc_rx;
  uint16_t crc_calc;

  if ((frame == 0) || (command_out == 0) || (frame_len < PUMP_CARD_CMD_FRAME_SIZE))
  {
    return PUMP_STATUS_BAD_LEN;
  }

  memset(command_out, 0, sizeof(*command_out));

  if (frame[0] != PUMP_CARD_PROTO_SOF)
  {
    return PUMP_STATUS_BAD_CMD;
  }

  payload_len = pump_card_read_u16_le(&frame[2]);
  if (payload_len > PUMP_CARD_CMD_PAYLOAD_MAX)
  {
    return PUMP_STATUS_BAD_LEN;
  }

  crc_rx = pump_card_read_u16_le(&frame[4 + payload_len]);
  crc_calc = pump_card_crc16_ccitt_false(&frame[1], (uint16_t)(3U + payload_len));
  if (crc_rx != crc_calc)
  {
    return PUMP_STATUS_BAD_CRC;
  }

  command_out->command_id = frame[1];
  command_out->payload_len = payload_len;
  if (payload_len > 0U)
  {
    memcpy(command_out->payload, &frame[4], payload_len);
  }

  return PUMP_STATUS_OK;
}

bool pump_card_encode_response_frame(uint8_t ack,
                                     uint8_t status,
                                     const uint8_t *payload,
                                     uint16_t payload_len,
                                     uint8_t *frame_out,
                                     uint16_t frame_out_size)
{
  uint16_t crc;

  if ((frame_out == 0) || (frame_out_size < PUMP_CARD_RESP_FRAME_SIZE))
  {
    return false;
  }

  if ((payload_len > PUMP_CARD_RESP_PAYLOAD_MAX) || ((payload_len > 0U) && (payload == 0)))
  {
    return false;
  }

  memset(frame_out, 0, frame_out_size);
  frame_out[0] = PUMP_CARD_PROTO_SOF;
  frame_out[1] = ack;
  frame_out[2] = status;
  pump_card_write_u16_le(&frame_out[3], payload_len);

  if (payload_len > 0U)
  {
    memcpy(&frame_out[5], payload, payload_len);
  }

  crc = pump_card_crc16_ccitt_false(&frame_out[1], (uint16_t)(4U + payload_len));
  pump_card_write_u16_le(&frame_out[5 + payload_len], crc);
  return true;
}

uint8_t pump_card_decode_response_frame(const uint8_t *frame,
                                        uint16_t frame_len,
                                        PumpCardResponse *response_out)
{
  uint16_t payload_len;
  uint16_t crc_rx;
  uint16_t crc_calc;

  if ((frame == 0) || (response_out == 0) || (frame_len < PUMP_CARD_RESP_FRAME_SIZE))
  {
    return PUMP_STATUS_BAD_LEN;
  }

  memset(response_out, 0, sizeof(*response_out));

  if (frame[0] != PUMP_CARD_PROTO_SOF)
  {
    return PUMP_STATUS_BAD_CMD;
  }

  payload_len = pump_card_read_u16_le(&frame[3]);
  if (payload_len > PUMP_CARD_RESP_PAYLOAD_MAX)
  {
    return PUMP_STATUS_BAD_LEN;
  }

  crc_rx = pump_card_read_u16_le(&frame[5 + payload_len]);
  crc_calc = pump_card_crc16_ccitt_false(&frame[1], (uint16_t)(4U + payload_len));
  if (crc_rx != crc_calc)
  {
    return PUMP_STATUS_BAD_CRC;
  }

  response_out->ack = frame[1];
  response_out->status = frame[2];
  response_out->payload_len = payload_len;
  if (payload_len > 0U)
  {
    memcpy(response_out->payload, &frame[5], payload_len);
  }

  return PUMP_STATUS_OK;
}

bool pump_card_build_load_event_payload(const PumpCardEvent *event,
                                        uint8_t *payload_out,
                                        uint16_t payload_out_size,
                                        uint16_t *payload_len_out)
{
  uint16_t payload_len;
  uint8_t action_index;
  uint16_t offset;

  if ((event == 0) || (payload_out == 0) || (payload_len_out == 0))
  {
    return false;
  }

  if (event->action_count > PUMP_CARD_MAX_ACTIONS_PER_EVENT)
  {
    return false;
  }

  payload_len = (uint16_t)(7U + ((uint16_t)event->action_count * 8U));
  if (payload_len > payload_out_size)
  {
    return false;
  }

  pump_card_write_u16_le(&payload_out[0], event->local_event_index);
  pump_card_write_u32_le(&payload_out[2], event->global_event_id);
  payload_out[6] = event->action_count;

  offset = 7U;
  for (action_index = 0U; action_index < event->action_count; action_index++)
  {
    payload_out[offset + 0U] = event->actions[action_index].local_pump_id;
    payload_out[offset + 1U] = event->actions[action_index].enable;
    payload_out[offset + 2U] = event->actions[action_index].direction;
    payload_out[offset + 3U] = event->actions[action_index].reserved;
    pump_card_write_u32_le(&payload_out[offset + 4U], event->actions[action_index].flow_ul_min);
    offset = (uint16_t)(offset + 8U);
  }

  *payload_len_out = payload_len;
  return true;
}

bool pump_card_parse_load_event_payload(const uint8_t *payload,
                                        uint16_t payload_len,
                                        PumpCardEvent *event_out)
{
  uint8_t action_index;
  uint16_t offset;

  if ((payload == 0) || (event_out == 0) || (payload_len < 7U))
  {
    return false;
  }

  memset(event_out, 0, sizeof(*event_out));
  event_out->local_event_index = pump_card_read_u16_le(&payload[0]);
  event_out->global_event_id = pump_card_read_u32_le(&payload[2]);
  event_out->action_count = payload[6];

  if (event_out->action_count > PUMP_CARD_MAX_ACTIONS_PER_EVENT)
  {
    return false;
  }

  if (payload_len != (uint16_t)(7U + ((uint16_t)event_out->action_count * 8U)))
  {
    return false;
  }

  offset = 7U;
  for (action_index = 0U; action_index < event_out->action_count; action_index++)
  {
    event_out->actions[action_index].local_pump_id = payload[offset + 0U];
    event_out->actions[action_index].enable = payload[offset + 1U];
    event_out->actions[action_index].direction = payload[offset + 2U];
    event_out->actions[action_index].reserved = payload[offset + 3U];
    event_out->actions[action_index].flow_ul_min = pump_card_read_u32_le(&payload[offset + 4U]);
    offset = (uint16_t)(offset + 8U);
  }

  return true;
}

void pump_card_build_exec_event_payload(uint16_t local_event_index,
                                        uint32_t global_event_id,
                                        uint8_t *payload_out,
                                        uint16_t *payload_len_out)
{
  if ((payload_out == 0) || (payload_len_out == 0))
  {
    return;
  }

  pump_card_write_u16_le(&payload_out[0], local_event_index);
  pump_card_write_u32_le(&payload_out[2], global_event_id);
  *payload_len_out = PUMP_CARD_EXEC_PAYLOAD_SIZE;
}

bool pump_card_parse_exec_event_payload(const uint8_t *payload,
                                        uint16_t payload_len,
                                        uint16_t *local_event_index,
                                        uint32_t *global_event_id)
{
  if ((payload == 0) || (local_event_index == 0) || (global_event_id == 0))
  {
    return false;
  }

  if (payload_len != PUMP_CARD_EXEC_PAYLOAD_SIZE)
  {
    return false;
  }

  *local_event_index = pump_card_read_u16_le(&payload[0]);
  *global_event_id = pump_card_read_u32_le(&payload[2]);
  return true;
}

void pump_card_build_identify_payload(const PumpCardIdentifyInfo *info,
                                      uint8_t *payload_out,
                                      uint16_t *payload_len_out)
{
  if ((info == 0) || (payload_out == 0) || (payload_len_out == 0))
  {
    return;
  }

  payload_out[0] = info->card_type;
  payload_out[1] = info->firmware_major;
  payload_out[2] = info->firmware_minor;
  pump_card_write_u16_le(&payload_out[3], info->capabilities);
  payload_out[5] = info->max_local_events;
  *payload_len_out = PUMP_CARD_IDENTIFY_PAYLOAD_SIZE;
}

bool pump_card_parse_identify_payload(const uint8_t *payload,
                                      uint16_t payload_len,
                                      PumpCardIdentifyInfo *info_out)
{
  if ((payload == 0) || (info_out == 0) || (payload_len != PUMP_CARD_IDENTIFY_PAYLOAD_SIZE))
  {
    return false;
  }

  info_out->card_type = payload[0];
  info_out->firmware_major = payload[1];
  info_out->firmware_minor = payload[2];
  info_out->capabilities = pump_card_read_u16_le(&payload[3]);
  info_out->max_local_events = payload[5];
  return true;
}

void pump_card_build_status_payload(const PumpCardStatusInfo *info,
                                    uint8_t *payload_out,
                                    uint16_t *payload_len_out)
{
  if ((info == 0) || (payload_out == 0) || (payload_len_out == 0))
  {
    return;
  }

  payload_out[0] = info->armed;
  pump_card_write_u16_le(&payload_out[1], info->event_count);
  pump_card_write_u16_le(&payload_out[3], info->next_event_index);
  pump_card_write_u32_le(&payload_out[5], info->last_global_event_id);
  *payload_len_out = PUMP_CARD_STATUS_PAYLOAD_SIZE;
}

bool pump_card_parse_status_payload(const uint8_t *payload,
                                    uint16_t payload_len,
                                    PumpCardStatusInfo *info_out)
{
  if ((payload == 0) || (info_out == 0) || (payload_len != PUMP_CARD_STATUS_PAYLOAD_SIZE))
  {
    return false;
  }

  info_out->armed = payload[0];
  info_out->event_count = pump_card_read_u16_le(&payload[1]);
  info_out->next_event_index = pump_card_read_u16_le(&payload[3]);
  info_out->last_global_event_id = pump_card_read_u32_le(&payload[5]);
  return true;
}
