#include "protocol.h"

#include <string.h>

typedef enum
{
  PROTOCOL_STATE_WAIT_SOF = 0U,
  PROTOCOL_STATE_VERSION,
  PROTOCOL_STATE_MSG_TYPE,
  PROTOCOL_STATE_SEQ_L,
  PROTOCOL_STATE_SEQ_H,
  PROTOCOL_STATE_LEN_L,
  PROTOCOL_STATE_LEN_H,
  PROTOCOL_STATE_PAYLOAD,
  PROTOCOL_STATE_CRC_L,
  PROTOCOL_STATE_CRC_H
} ProtocolParserState;

static uint16_t protocol_crc16_update(uint16_t crc, uint8_t byte)
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

static void protocol_reset_frame(ProtocolParser *parser)
{
  parser->state = PROTOCOL_STATE_WAIT_SOF;
  parser->payload_index = 0U;
  parser->crc_calculated = 0xFFFFU;
  parser->crc_received = 0U;
  memset(&parser->current_packet, 0, sizeof(parser->current_packet));
}

static void protocol_emit_parser_error(ProtocolParser *parser, uint16_t seq, uint8_t error_code, uint8_t detail)
{
  if (parser->error_handler != 0)
  {
    parser->error_handler(seq, error_code, detail, parser->context);
  }
}

uint16_t protocol_crc16_ccitt_false(const uint8_t *data, uint16_t len)
{
  uint16_t crc = 0xFFFFU;
  uint16_t index;

  for (index = 0U; index < len; index++)
  {
    crc = protocol_crc16_update(crc, data[index]);
  }

  return crc;
}

uint16_t read_u16_le(const uint8_t *data)
{
  return (uint16_t)data[0] | ((uint16_t)data[1] << 8);
}

uint32_t read_u32_le(const uint8_t *data)
{
  return (uint32_t)data[0]
       | ((uint32_t)data[1] << 8)
       | ((uint32_t)data[2] << 16)
       | ((uint32_t)data[3] << 24);
}

uint64_t read_u64_le(const uint8_t *data)
{
  return (uint64_t)data[0]
       | ((uint64_t)data[1] << 8)
       | ((uint64_t)data[2] << 16)
       | ((uint64_t)data[3] << 24)
       | ((uint64_t)data[4] << 32)
       | ((uint64_t)data[5] << 40)
       | ((uint64_t)data[6] << 48)
       | ((uint64_t)data[7] << 56);
}

int32_t read_i32_le(const uint8_t *data)
{
  return (int32_t)read_u32_le(data);
}

void write_u16_le(uint8_t *dst, uint16_t value)
{
  dst[0] = (uint8_t)(value & 0xFFU);
  dst[1] = (uint8_t)((value >> 8) & 0xFFU);
}

void write_u32_le(uint8_t *dst, uint32_t value)
{
  dst[0] = (uint8_t)(value & 0xFFU);
  dst[1] = (uint8_t)((value >> 8) & 0xFFU);
  dst[2] = (uint8_t)((value >> 16) & 0xFFU);
  dst[3] = (uint8_t)((value >> 24) & 0xFFU);
}

void write_u64_le(uint8_t *dst, uint64_t value)
{
  dst[0] = (uint8_t)(value & 0xFFU);
  dst[1] = (uint8_t)((value >> 8) & 0xFFU);
  dst[2] = (uint8_t)((value >> 16) & 0xFFU);
  dst[3] = (uint8_t)((value >> 24) & 0xFFU);
  dst[4] = (uint8_t)((value >> 32) & 0xFFU);
  dst[5] = (uint8_t)((value >> 40) & 0xFFU);
  dst[6] = (uint8_t)((value >> 48) & 0xFFU);
  dst[7] = (uint8_t)((value >> 56) & 0xFFU);
}

void protocol_parser_init(ProtocolParser *parser,
                          ProtocolPacketHandler packet_handler,
                          ProtocolParserErrorHandler error_handler,
                          void *context)
{
  if (parser == 0)
  {
    return;
  }

  memset(parser, 0, sizeof(*parser));
  parser->packet_handler = packet_handler;
  parser->error_handler = error_handler;
  parser->context = context;
  protocol_reset_frame(parser);
}

void protocol_parser_process(ProtocolParser *parser, const uint8_t *data, uint32_t len)
{
  uint32_t index;

  if ((parser == 0) || (data == 0))
  {
    return;
  }

  for (index = 0U; index < len; index++)
  {
    uint8_t byte = data[index];

    switch ((ProtocolParserState)parser->state)
    {
      case PROTOCOL_STATE_WAIT_SOF:
        if (byte == PROTOCOL_SOF)
        {
          protocol_reset_frame(parser);
          parser->state = PROTOCOL_STATE_VERSION;
        }
        break;

      case PROTOCOL_STATE_VERSION:
        parser->current_packet.version = byte;
        parser->crc_calculated = protocol_crc16_update(0xFFFFU, byte);
        parser->state = PROTOCOL_STATE_MSG_TYPE;
        break;

      case PROTOCOL_STATE_MSG_TYPE:
        parser->current_packet.msg_type = byte;
        parser->crc_calculated = protocol_crc16_update(parser->crc_calculated, byte);
        parser->state = PROTOCOL_STATE_SEQ_L;
        break;

      case PROTOCOL_STATE_SEQ_L:
        parser->current_packet.seq = byte;
        parser->crc_calculated = protocol_crc16_update(parser->crc_calculated, byte);
        parser->state = PROTOCOL_STATE_SEQ_H;
        break;

      case PROTOCOL_STATE_SEQ_H:
        parser->current_packet.seq |= (uint16_t)byte << 8;
        parser->crc_calculated = protocol_crc16_update(parser->crc_calculated, byte);
        parser->state = PROTOCOL_STATE_LEN_L;
        break;

      case PROTOCOL_STATE_LEN_L:
        parser->current_packet.payload_len = byte;
        parser->crc_calculated = protocol_crc16_update(parser->crc_calculated, byte);
        parser->state = PROTOCOL_STATE_LEN_H;
        break;

      case PROTOCOL_STATE_LEN_H:
        parser->current_packet.payload_len |= (uint16_t)byte << 8;
        parser->crc_calculated = protocol_crc16_update(parser->crc_calculated, byte);

        if (parser->current_packet.version != PROTOCOL_VERSION)
        {
          protocol_emit_parser_error(parser,
                                     parser->current_packet.seq,
                                     ERR_BAD_VERSION,
                                     parser->current_packet.version);
          protocol_reset_frame(parser);
        }
        else if (parser->current_packet.payload_len > PROTOCOL_MAX_PAYLOAD)
        {
          protocol_emit_parser_error(parser,
                                     parser->current_packet.seq,
                                     ERR_BAD_LENGTH,
                                     (uint8_t)(parser->current_packet.payload_len & 0xFFU));
          protocol_reset_frame(parser);
        }
        else if (parser->current_packet.payload_len == 0U)
        {
          parser->state = PROTOCOL_STATE_CRC_L;
        }
        else
        {
          parser->state = PROTOCOL_STATE_PAYLOAD;
        }
        break;

      case PROTOCOL_STATE_PAYLOAD:
        parser->current_packet.payload[parser->payload_index++] = byte;
        parser->crc_calculated = protocol_crc16_update(parser->crc_calculated, byte);

        if (parser->payload_index >= parser->current_packet.payload_len)
        {
          parser->state = PROTOCOL_STATE_CRC_L;
        }
        break;

      case PROTOCOL_STATE_CRC_L:
        parser->crc_received = byte;
        parser->state = PROTOCOL_STATE_CRC_H;
        break;

      case PROTOCOL_STATE_CRC_H:
        parser->crc_received |= (uint16_t)byte << 8;

        if (parser->crc_received == parser->crc_calculated)
        {
          if (parser->packet_handler != 0)
          {
            parser->packet_handler(&parser->current_packet, parser->context);
          }
        }
        else
        {
          protocol_emit_parser_error(parser, parser->current_packet.seq, ERR_BAD_CRC, 0U);
        }

        protocol_reset_frame(parser);
        break;

      default:
        protocol_reset_frame(parser);
        break;
    }
  }
}

uint16_t protocol_encode_frame(uint8_t msg_type,
                               uint16_t seq,
                               const uint8_t *payload,
                               uint16_t payload_len,
                               uint8_t *frame_out,
                               uint16_t frame_out_size)
{
  uint16_t crc;
  uint16_t frame_len;

  if ((frame_out == 0) || (payload_len > PROTOCOL_MAX_PAYLOAD))
  {
    return 0U;
  }

  if ((payload_len > 0U) && (payload == 0))
  {
    return 0U;
  }

  frame_len = (uint16_t)(PROTOCOL_FRAME_OVERHEAD + payload_len);
  if (frame_out_size < frame_len)
  {
    return 0U;
  }

  frame_out[0] = PROTOCOL_SOF;
  frame_out[1] = PROTOCOL_VERSION;
  frame_out[2] = msg_type;
  write_u16_le(&frame_out[3], seq);
  write_u16_le(&frame_out[5], payload_len);

  if ((payload_len > 0U) && (payload != 0))
  {
    memcpy(&frame_out[7], payload, payload_len);
  }

  crc = protocol_crc16_ccitt_false(&frame_out[1], (uint16_t)(6U + payload_len));
  write_u16_le(&frame_out[7 + payload_len], crc);

  return frame_len;
}

uint8_t protocol_send_frame(ProtocolTransmitFn tx_fn,
                            void *tx_context,
                            uint8_t msg_type,
                            uint16_t seq,
                            const uint8_t *payload,
                            uint16_t payload_len)
{
  uint8_t frame[PROTOCOL_MAX_FRAME_SIZE];
  uint16_t frame_len;

  if (tx_fn == 0)
  {
    return 1U;
  }

  frame_len = protocol_encode_frame(msg_type, seq, payload, payload_len, frame, sizeof(frame));
  if (frame_len == 0U)
  {
    return 1U;
  }

  return tx_fn(frame, frame_len, tx_context);
}

uint8_t protocol_send_ack(ProtocolTransmitFn tx_fn,
                          void *tx_context,
                          uint16_t ack_seq,
                          uint8_t status)
{
  uint8_t payload[3];

  write_u16_le(payload, ack_seq);
  payload[2] = status;

  return protocol_send_frame(tx_fn, tx_context, MSG_ACK, ack_seq, payload, sizeof(payload));
}

uint8_t protocol_send_error(ProtocolTransmitFn tx_fn,
                            void *tx_context,
                            uint16_t failed_seq,
                            uint8_t error_code,
                            uint8_t detail)
{
  uint8_t payload[4];

  write_u16_le(payload, failed_seq);
  payload[2] = error_code;
  payload[3] = detail;

  return protocol_send_frame(tx_fn, tx_context, MSG_ERROR, failed_seq, payload, sizeof(payload));
}
