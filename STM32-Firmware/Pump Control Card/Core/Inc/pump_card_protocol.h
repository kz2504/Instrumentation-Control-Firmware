#ifndef __PUMP_CARD_PROTOCOL_H__
#define __PUMP_CARD_PROTOCOL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#define PUMP_CARD_SLOT_COUNT             8U
#define PUMP_CARD_PUMPS_PER_CARD         8U
#define PUMP_CARD_MAX_ACTIONS_PER_EVENT  8U
#define PUMP_CARD_MAX_LOCAL_EVENTS       48U

#define PUMP_CARD_PROTO_SOF              0xC3U
#define PUMP_CARD_CMD_FRAME_SIZE         80U
#define PUMP_CARD_RESP_FRAME_SIZE        20U
#define PUMP_CARD_CMD_PAYLOAD_MAX        74U
#define PUMP_CARD_RESP_PAYLOAD_MAX       13U

#define PUMP_CARD_TYPE_PERISTALTIC       0x01U
#define PUMP_CARD_CAP_PERISTALTIC        0x0001U
#define PUMP_CARD_FW_MAJOR               0x01U
#define PUMP_CARD_FW_MINOR               0x00U

#define PUMP_CARD_IDENTIFY_PAYLOAD_SIZE  6U
#define PUMP_CARD_EXEC_PAYLOAD_SIZE      6U
#define PUMP_CARD_STATUS_PAYLOAD_SIZE    9U

typedef enum
{
  CMD_IDENTIFY = 0x01U,
  CMD_CLEAR = 0x02U,
  CMD_LOAD_EVENT = 0x03U,
  CMD_ARM = 0x04U,
  CMD_EXEC_EVENT = 0x05U,
  CMD_STOP = 0x06U,
  CMD_STATUS = 0x07U
} PumpCardCommandId;

typedef enum
{
  PUMP_ACK_OK = 0x79U,
  PUMP_ACK_ERR = 0x1FU
} PumpCardAck;

typedef enum
{
  PUMP_STATUS_OK = 0x00U,
  PUMP_STATUS_BAD_CMD = 0x01U,
  PUMP_STATUS_BAD_LEN = 0x02U,
  PUMP_STATUS_BAD_CRC = 0x03U,
  PUMP_STATUS_NOT_ARMED = 0x04U,
  PUMP_STATUS_BAD_INDEX = 0x05U,
  PUMP_STATUS_FULL = 0x06U,
  PUMP_STATUS_HW_ERROR = 0x07U
} PumpCardStatusCode;

typedef struct
{
  uint8_t local_pump_id;
  uint8_t enable;
  uint8_t direction;
  uint8_t reserved;
  uint32_t flow_ul_min;
} PumpAction;

typedef struct
{
  uint16_t local_event_index;
  uint32_t global_event_id;
  uint8_t action_count;
  PumpAction actions[PUMP_CARD_MAX_ACTIONS_PER_EVENT];
} PumpCardEvent;

typedef struct
{
  uint8_t command_id;
  uint16_t payload_len;
  uint8_t payload[PUMP_CARD_CMD_PAYLOAD_MAX];
} PumpCardCommand;

typedef struct
{
  uint8_t ack;
  uint8_t status;
  uint16_t payload_len;
  uint8_t payload[PUMP_CARD_RESP_PAYLOAD_MAX];
} PumpCardResponse;

typedef struct
{
  uint8_t card_type;
  uint8_t firmware_major;
  uint8_t firmware_minor;
  uint16_t capabilities;
  uint8_t max_local_events;
} PumpCardIdentifyInfo;

typedef struct
{
  uint8_t armed;
  uint16_t event_count;
  uint16_t next_event_index;
  uint32_t last_global_event_id;
} PumpCardStatusInfo;

uint16_t pump_card_crc16_ccitt_false(const uint8_t *data, uint16_t len);

uint16_t pump_card_read_u16_le(const uint8_t *data);
uint32_t pump_card_read_u32_le(const uint8_t *data);
void pump_card_write_u16_le(uint8_t *dst, uint16_t value);
void pump_card_write_u32_le(uint8_t *dst, uint32_t value);

bool pump_card_encode_command_frame(uint8_t command_id,
                                    const uint8_t *payload,
                                    uint16_t payload_len,
                                    uint8_t *frame_out,
                                    uint16_t frame_out_size);
uint8_t pump_card_decode_command_frame(const uint8_t *frame,
                                       uint16_t frame_len,
                                       PumpCardCommand *command_out);

bool pump_card_encode_response_frame(uint8_t ack,
                                     uint8_t status,
                                     const uint8_t *payload,
                                     uint16_t payload_len,
                                     uint8_t *frame_out,
                                     uint16_t frame_out_size);
uint8_t pump_card_decode_response_frame(const uint8_t *frame,
                                        uint16_t frame_len,
                                        PumpCardResponse *response_out);

bool pump_card_build_load_event_payload(const PumpCardEvent *event,
                                        uint8_t *payload_out,
                                        uint16_t payload_out_size,
                                        uint16_t *payload_len_out);
bool pump_card_parse_load_event_payload(const uint8_t *payload,
                                        uint16_t payload_len,
                                        PumpCardEvent *event_out);

void pump_card_build_exec_event_payload(uint16_t local_event_index,
                                        uint32_t global_event_id,
                                        uint8_t *payload_out,
                                        uint16_t *payload_len_out);
bool pump_card_parse_exec_event_payload(const uint8_t *payload,
                                        uint16_t payload_len,
                                        uint16_t *local_event_index,
                                        uint32_t *global_event_id);

void pump_card_build_identify_payload(const PumpCardIdentifyInfo *info,
                                      uint8_t *payload_out,
                                      uint16_t *payload_len_out);
bool pump_card_parse_identify_payload(const uint8_t *payload,
                                      uint16_t payload_len,
                                      PumpCardIdentifyInfo *info_out);

void pump_card_build_status_payload(const PumpCardStatusInfo *info,
                                    uint8_t *payload_out,
                                    uint16_t *payload_len_out);
bool pump_card_parse_status_payload(const uint8_t *payload,
                                    uint16_t payload_len,
                                    PumpCardStatusInfo *info_out);

#ifdef __cplusplus
}
#endif

#endif /* __PUMP_CARD_PROTOCOL_H__ */
