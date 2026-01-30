#include "main.h"
#include "interface.h"
#include "can.h"

void CANfilter(void)
{
  CAN_FilterTypeDef filter = {0};

  filter.FilterBank = 0;
  filter.FilterMode = CAN_FILTERMODE_IDMASK;
  filter.FilterScale = CAN_FILTERSCALE_32BIT;
  filter.FilterIdHigh = 0x0000;
  filter.FilterIdLow  = 0x0000;
  filter.FilterMaskIdHigh = 0x0000;
  filter.FilterMaskIdLow  = 0x0000;
  filter.FilterFIFOAssignment = CAN_FILTER_FIFO0;
  filter.FilterActivation = ENABLE;

  HAL_CAN_ConfigFilter(&hcan, &filter);
}
