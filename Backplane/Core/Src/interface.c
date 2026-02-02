#include "main.h"
#include "interface.h"
#include "spi.h"

uint8_t SPI_TIMEOUT = 100;

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

int transmitSPI1(uint8_t card, uint8_t* data, uint16_t len) {
	setCS(card);
	HAL_StatusTypeDef status = HAL_SPI_Transmit(&hspi1, data, len, SPI_TIMEOUT);
	resetCS();
	uint8_t ok = (status == HAL_OK) ? 1 : 0;
	return ok;
}



