#include "main.h"
#include "DAC.h"

void setCS(uint8_t line) {
	HAL_GPIO_WritePin(EN_CS_GPIO_Port, EN_CS_Pin, GPIO_PIN_SET);
	uint8_t A0 = (line >> 0) & 1;
	uint8_t A1 = (line >> 1) & 1;
	uint8_t A2 = (line >> 2) & 1;
	HAL_GPIO_WritePin(A0_CS_GPIO_Port, A0_CS_Pin, A0 ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(A1_CS_GPIO_Port, A1_CS_Pin, A1 ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(A2_CS_GPIO_Port, A2_CS_Pin, A2 ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(EN_CS_GPIO_Port, EN_CS_Pin, GPIO_PIN_RESET);
}

void resetCS(void) {
	HAL_GPIO_WritePin(EN_CS_GPIO_Port, EN_CS_Pin, GPIO_PIN_SET);
}

void writeDAC(uint8_t channel, float AVDD, float vOut, uint8_t mode) {
	float F = vOut / AVDD * 4096.0f;
	if (F <= 0.0f){
		F = 0.0f;
	}
	if (F >= 4095.0f){
		F = 4095.0f;
	}
	uint16_t D = (uint16_t)F;
	uint16_t W = ((mode & 0x3) << 14) | ((D & 0x0FFF) << 2);
	setCS(channel);
	HAL_GPIO_WritePin(SCLK_DAC_GPIO_Port, SCLK_DAC_Pin, GPIO_PIN_RESET);
	for (int i = 15; i >= 0; i--) {
		GPIO_PinState din = (W & (1u << i)) ? GPIO_PIN_SET : GPIO_PIN_RESET;
		HAL_GPIO_WritePin(DIN_DAC_GPIO_Port, DIN_DAC_Pin, din);
		HAL_GPIO_WritePin(SCLK_DAC_GPIO_Port, SCLK_DAC_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(SCLK_DAC_GPIO_Port, SCLK_DAC_Pin, GPIO_PIN_RESET);
	}
	HAL_GPIO_WritePin(DIN_DAC_GPIO_Port, DIN_DAC_Pin, GPIO_PIN_RESET);
	resetCS();
}
