#ifndef __BACKPLANE_CARD_BUS_H__
#define __BACKPLANE_CARD_BUS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#include "card_registers.h"

typedef struct
{
  uint8_t present;
  uint8_t card_type;
  uint8_t firmware_major;
  uint8_t firmware_minor;
  uint16_t capabilities;
  uint8_t max_local_events;
} CardSlotInfo;

void card_bus_init(void);
void card_bus_discover_all(void);
bool card_bus_discover_type_slots(uint8_t slot_mask, uint8_t required_card_type, uint8_t pass_count);
bool card_bus_is_slot_present(uint8_t slot, uint8_t required_card_type);
uint8_t card_bus_find_first_slot(uint8_t required_card_type);
const CardSlotInfo *card_bus_get_slots(void);

bool card_bus_read_reg(uint8_t slot, uint16_t reg_addr, uint32_t *value_out);
bool card_bus_write_reg(uint8_t slot, uint16_t reg_addr, uint32_t value);

uint8_t card_bus_get_last_status(void);
uint8_t card_bus_get_last_detail(void);

#ifdef __cplusplus
}
#endif

#endif /* __BACKPLANE_CARD_BUS_H__ */
