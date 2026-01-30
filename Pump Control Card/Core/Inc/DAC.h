#pragma once
#include "main.h"

void setCS(uint8_t line);
void resetCS(void);
void writeDAC(uint8_t channel, float AVDD, float vOut, uint8_t mode);
