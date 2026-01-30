#include "main.h"
#include "interface.h"
#include "can.h"

void setCS(uint8_t line) {
	line = 7u - line;
	uint8_t A0 = (line >> 0) & 1;
	uint8_t A1 = (line >> 1) & 1;
	uint8_t A2 = (line >> 2) & 1;
	HAL_GPIO_WritePin(EN_CS_GPIO_Port, EN_CS_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(A0_CS_GPIO_Port, A0_CS_Pin, A0 ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(A1_CS_GPIO_Port, A1_CS_Pin, A1 ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(A2_CS_GPIO_Port, A2_CS_Pin, A2 ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(EN_CS_GPIO_Port, EN_CS_Pin, GPIO_PIN_RESET);
}

void resetCS(void) {
	HAL_GPIO_WritePin(EN_CS_GPIO_Port, EN_CS_Pin, GPIO_PIN_SET);
}

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
