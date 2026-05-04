#ifndef __SPI_FRAME_H__
#define __SPI_FRAME_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#define SPI_FRAME_SOF                0xC3U
#define SPI_FRAME_CMD_SIZE           80U
#define SPI_FRAME_RESP_SIZE          20U
#define SPI_FRAME_CMD_PAYLOAD_MAX    74U
#define SPI_FRAME_RESP_PAYLOAD_MAX   13U

#define SPI_FRAME_ACK_OK             0x79U
#define SPI_FRAME_ACK_ERR            0x1FU

#define CMD_IDENTIFY                 0x01U
#define CMD_READ_REG                 0x02U
#define CMD_WRITE_REG                0x03U

#define STATUS_OK                    0x00U
#define STATUS_BAD_CMD               0x01U
#define STATUS_BAD_LEN               0x02U
#define STATUS_BAD_CRC               0x03U
#define STATUS_BAD_ADDR              0x04U
#define STATUS_HW_ERROR              0x07U

typedef struct
{
  uint8_t command_id;
  uint16_t payload_len;
  uint8_t payload[SPI_FRAME_CMD_PAYLOAD_MAX];
} SpiFrameCommand;

typedef struct
{
  uint8_t ack;
  uint8_t status;
  uint16_t payload_len;
  uint8_t payload[SPI_FRAME_RESP_PAYLOAD_MAX];
} SpiFrameResponse;

uint16_t spi_frame_crc16_ccitt_false(const uint8_t *data, uint16_t len);

uint16_t spi_frame_read_u16_le(const uint8_t *data);
uint32_t spi_frame_read_u32_le(const uint8_t *data);
void spi_frame_write_u16_le(uint8_t *dst, uint16_t value);
void spi_frame_write_u32_le(uint8_t *dst, uint32_t value);

bool spi_frame_build_command(uint8_t command_id,
                             const uint8_t *payload,
                             uint16_t payload_len,
                             uint8_t *frame_out,
                             uint16_t frame_out_size);
uint8_t spi_frame_parse_command(const uint8_t *frame,
                                uint16_t frame_len,
                                SpiFrameCommand *command_out);

bool spi_frame_build_response(uint8_t ack,
                              uint8_t status,
                              const uint8_t *payload,
                              uint16_t payload_len,
                              uint8_t *frame_out,
                              uint16_t frame_out_size);
uint8_t spi_frame_parse_response(const uint8_t *frame,
                                 uint16_t frame_len,
                                 SpiFrameResponse *response_out);

#ifdef __cplusplus
}
#endif

#endif /* __SPI_FRAME_H__ */
