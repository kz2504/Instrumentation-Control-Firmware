#pragma once
#include "main.h"
#include "can.h"

void CANfilter(void);
int transmitSPI1(uint8_t* data, uint16_t len);
