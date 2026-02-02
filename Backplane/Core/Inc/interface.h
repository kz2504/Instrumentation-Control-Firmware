#pragma once
#include "main.h"

void setCS(uint8_t line);
void resetCS(void);
int transmitSPI1(uint8_t card, uint8_t* data, uint16_t len);
