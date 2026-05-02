#ifndef __BACKPLANE_CARD_BUS_H__
#define __BACKPLANE_CARD_BUS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#include "pump_card_protocol.h"

#define CARD_BUS_SLOT_COUNT            PUMP_CARD_SLOT_COUNT
#define CARD_BUS_ALL_SLOTS_MASK        0xFFU
#define CARD_BUS_RESPONSE_PAYLOAD_MAX  PUMP_CARD_RESP_PAYLOAD_MAX

typedef enum
{
  CARD_TYPE_NONE = 0x00U,
  CARD_TYPE_PUMP_PERISTALTIC = PUMP_CARD_TYPE_PERISTALTIC
} CardType;

typedef struct
{
  uint8_t present;
  uint8_t card_type;
  uint8_t firmware_major;
  uint8_t firmware_minor;
  uint16_t capabilities;
  uint8_t max_local_events;
} CardSlotInfo;

typedef struct
{
  uint8_t ack;
  uint8_t status;
  uint16_t payload_len;
  uint8_t payload[CARD_BUS_RESPONSE_PAYLOAD_MAX];
} CardBusResponse;

void card_bus_init(void);
void card_bus_discover_all(void);
bool card_bus_discover_type_slots(uint8_t slot_mask, uint8_t required_card_type, uint8_t pass_count);
bool card_bus_is_slot_present(uint8_t slot, uint8_t required_card_type);
const CardSlotInfo *card_bus_get_slots(void);

bool card_bus_command(uint8_t slot,
                      uint8_t command_id,
                      const uint8_t *payload,
                      uint16_t payload_len,
                      CardBusResponse *response_out);

uint8_t card_bus_get_last_status(void);
uint8_t card_bus_get_last_detail(void);

#ifdef __cplusplus
}
#endif

#endif /* __BACKPLANE_CARD_BUS_H__ */
