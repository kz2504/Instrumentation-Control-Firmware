#include "spi_frame.h"

#include <string.h>

static uint16_t spi_frame_crc16_update(uint16_t crc, uint8_t byte)
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

uint16_t spi_frame_crc16_ccitt_false(const uint8_t *data, uint16_t len)
{
  uint16_t crc = 0xFFFFU;
  uint16_t index;

  for (index = 0U; index < len; index++)
  {
    crc = spi_frame_crc16_update(crc, data[index]);
  }

  return crc;
}

uint16_t spi_frame_read_u16_le(const uint8_t *data)
{
  return (uint16_t)data[0] | ((uint16_t)data[1] << 8);
}

uint32_t spi_frame_read_u32_le(const uint8_t *data)
{
  return (uint32_t)data[0]
       | ((uint32_t)data[1] << 8)
       | ((uint32_t)data[2] << 16)
       | ((uint32_t)data[3] << 24);
}

void spi_frame_write_u16_le(uint8_t *dst, uint16_t value)
{
  dst[0] = (uint8_t)(value & 0xFFU);
  dst[1] = (uint8_t)((value >> 8) & 0xFFU);
}

void spi_frame_write_u32_le(uint8_t *dst, uint32_t value)
{
  dst[0] = (uint8_t)(value & 0xFFU);
  dst[1] = (uint8_t)((value >> 8) & 0xFFU);
  dst[2] = (uint8_t)((value >> 16) & 0xFFU);
  dst[3] = (uint8_t)((value >> 24) & 0xFFU);
}

bool spi_frame_build_command(uint8_t command_id,
                             const uint8_t *payload,
                             uint16_t payload_len,
                             uint8_t *frame_out,
                             uint16_t frame_out_size)
{
  uint16_t crc;

  if ((frame_out == 0) || (frame_out_size < SPI_FRAME_CMD_SIZE))
  {
    return false;
  }

  if ((payload_len > SPI_FRAME_CMD_PAYLOAD_MAX) || ((payload_len > 0U) && (payload == 0)))
  {
    return false;
  }

  memset(frame_out, 0, frame_out_size);
  frame_out[0] = SPI_FRAME_SOF;
  frame_out[1] = command_id;
  spi_frame_write_u16_le(&frame_out[2], payload_len);

  if (payload_len > 0U)
  {
    memcpy(&frame_out[4], payload, payload_len);
  }

  crc = spi_frame_crc16_ccitt_false(&frame_out[1], (uint16_t)(3U + payload_len));
  spi_frame_write_u16_le(&frame_out[4 + payload_len], crc);
  return true;
}

uint8_t spi_frame_parse_command(const uint8_t *frame,
                                uint16_t frame_len,
                                SpiFrameCommand *command_out)
{
  uint16_t payload_len;
  uint16_t crc_rx;
  uint16_t crc_calc;

  if ((frame == 0) || (command_out == 0) || (frame_len < SPI_FRAME_CMD_SIZE))
  {
    return STATUS_BAD_LEN;
  }

  memset(command_out, 0, sizeof(*command_out));

  if (frame[0] != SPI_FRAME_SOF)
  {
    return STATUS_BAD_CMD;
  }

  payload_len = spi_frame_read_u16_le(&frame[2]);
  if (payload_len > SPI_FRAME_CMD_PAYLOAD_MAX)
  {
    return STATUS_BAD_LEN;
  }

  crc_rx = spi_frame_read_u16_le(&frame[4 + payload_len]);
  crc_calc = spi_frame_crc16_ccitt_false(&frame[1], (uint16_t)(3U + payload_len));
  if (crc_rx != crc_calc)
  {
    return STATUS_BAD_CRC;
  }

  command_out->command_id = frame[1];
  command_out->payload_len = payload_len;
  if (payload_len > 0U)
  {
    memcpy(command_out->payload, &frame[4], payload_len);
  }

  return STATUS_OK;
}

bool spi_frame_build_response(uint8_t ack,
                              uint8_t status,
                              const uint8_t *payload,
                              uint16_t payload_len,
                              uint8_t *frame_out,
                              uint16_t frame_out_size)
{
  uint16_t crc;

  if ((frame_out == 0) || (frame_out_size < SPI_FRAME_RESP_SIZE))
  {
    return false;
  }

  if ((payload_len > SPI_FRAME_RESP_PAYLOAD_MAX) || ((payload_len > 0U) && (payload == 0)))
  {
    return false;
  }

  memset(frame_out, 0, frame_out_size);
  frame_out[0] = SPI_FRAME_SOF;
  frame_out[1] = ack;
  frame_out[2] = status;
  spi_frame_write_u16_le(&frame_out[3], payload_len);

  if (payload_len > 0U)
  {
    memcpy(&frame_out[5], payload, payload_len);
  }

  crc = spi_frame_crc16_ccitt_false(&frame_out[1], (uint16_t)(4U + payload_len));
  spi_frame_write_u16_le(&frame_out[5 + payload_len], crc);
  return true;
}

uint8_t spi_frame_parse_response(const uint8_t *frame,
                                 uint16_t frame_len,
                                 SpiFrameResponse *response_out)
{
  uint16_t payload_len;
  uint16_t crc_rx;
  uint16_t crc_calc;

  if ((frame == 0) || (response_out == 0) || (frame_len < SPI_FRAME_RESP_SIZE))
  {
    return STATUS_BAD_LEN;
  }

  memset(response_out, 0, sizeof(*response_out));

  if (frame[0] != SPI_FRAME_SOF)
  {
    return STATUS_BAD_CMD;
  }

  payload_len = spi_frame_read_u16_le(&frame[3]);
  if (payload_len > SPI_FRAME_RESP_PAYLOAD_MAX)
  {
    return STATUS_BAD_LEN;
  }

  crc_rx = spi_frame_read_u16_le(&frame[5 + payload_len]);
  crc_calc = spi_frame_crc16_ccitt_false(&frame[1], (uint16_t)(4U + payload_len));
  if (crc_rx != crc_calc)
  {
    return STATUS_BAD_CRC;
  }

  response_out->ack = frame[1];
  response_out->status = frame[2];
  response_out->payload_len = payload_len;
  if (payload_len > 0U)
  {
    memcpy(response_out->payload, &frame[5], payload_len);
  }

  return STATUS_OK;
}
