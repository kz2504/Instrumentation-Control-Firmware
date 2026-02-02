#pragma once
#include <stdint.h>

#define PUMP_MAX_PAYLOAD 255

//Debug watch vars
extern volatile uint32_t g_spi_frame_count;
extern volatile uint32_t g_spi_error_count;
extern volatile uint8_t  g_spi_last_cmd;
extern volatile uint8_t  g_spi_last_len;
extern volatile uint8_t  g_spi_last_payload[PUMP_MAX_PAYLOAD];

void pump_spi_nack_init(void);
void pump_spi_nack_poll(void);
