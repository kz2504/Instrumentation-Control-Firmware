#ifndef __BACKPLANE_FPGA_BUS_H__
#define __BACKPLANE_FPGA_BUS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#include "scheduler.h"

void fpga_bus_init(void);
bool fpga_bus_validate_schedule(const Schedule *schedule);
bool fpga_bus_start_schedule(const Schedule *schedule);
bool fpga_bus_arm_schedule(void);
bool fpga_bus_is_schedule_complete(void);
void fpga_bus_stop_all(void);

uint8_t fpga_bus_get_last_status(void);
uint8_t fpga_bus_get_last_detail(void);

#ifdef __cplusplus
}
#endif

#endif /* __BACKPLANE_FPGA_BUS_H__ */
