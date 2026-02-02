#pragma once
#include "main.h"
#include <stdint.h>

void setCS(uint8_t line);
void resetCS(void);

int spi_send_clear(uint8_t card);
int spi_send_start(uint8_t card);
int spi_send_stop(uint8_t card);
int spi_send_trigger_ready(uint8_t card, uint8_t ready);
int spi_send_block(uint8_t card, const void *blk, uint16_t len);
