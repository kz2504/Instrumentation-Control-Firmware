#include "main.h"
#include "shiftreg.h"

void shiftByteEN(uint8_t byte) {
	HAL_GPIO_WritePin(SRCLR_EN_GPIO_Port, SRCLR_EN_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(RCLK_EN_GPIO_Port, RCLK_EN_Pin, GPIO_PIN_RESET);

	for (int i = 0; i < 8; i++) {
		uint8_t out = byte & 0x80;
		HAL_GPIO_WritePin(SER_EN_GPIO_Port, SER_EN_Pin, out ? GPIO_PIN_SET : GPIO_PIN_RESET);
		HAL_GPIO_WritePin(SRCLK_EN_GPIO_Port, SRCLK_EN_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(SRCLK_EN_GPIO_Port, SRCLK_EN_Pin, GPIO_PIN_RESET);
		byte = byte << 1;
	}
	HAL_GPIO_WritePin(RCLK_EN_GPIO_Port, RCLK_EN_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(RCLK_EN_GPIO_Port, RCLK_EN_Pin, GPIO_PIN_RESET);

	HAL_GPIO_WritePin(SER_EN_GPIO_Port, SER_EN_Pin, GPIO_PIN_RESET);
}

void shiftByteDIR(uint8_t byte) {
	HAL_GPIO_WritePin(SRCLR_DIR_GPIO_Port, SRCLR_DIR_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(RCLK_DIR_GPIO_Port, RCLK_DIR_Pin, GPIO_PIN_RESET);

	for (int i = 0; i < 8; i++) {
		uint8_t out = byte & 0x80;
		HAL_GPIO_WritePin(SER_DIR_GPIO_Port, SER_DIR_Pin, out ? GPIO_PIN_SET : GPIO_PIN_RESET);
		HAL_GPIO_WritePin(SRCLK_DIR_GPIO_Port, SRCLK_DIR_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(SRCLK_DIR_GPIO_Port, SRCLK_DIR_Pin, GPIO_PIN_RESET);
		byte = byte << 1;
	}
	HAL_GPIO_WritePin(RCLK_DIR_GPIO_Port, RCLK_DIR_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(RCLK_DIR_GPIO_Port, RCLK_DIR_Pin, GPIO_PIN_RESET);

	HAL_GPIO_WritePin(SER_DIR_GPIO_Port, SER_DIR_Pin, GPIO_PIN_RESET);
}

void clearEN(void) {
	HAL_GPIO_WritePin(SRCLR_EN_GPIO_Port, SRCLR_EN_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(SRCLR_EN_GPIO_Port, SRCLR_EN_Pin, GPIO_PIN_SET);

	HAL_GPIO_WritePin(RCLK_DIR_GPIO_Port, RCLK_DIR_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(RCLK_DIR_GPIO_Port, RCLK_DIR_Pin, GPIO_PIN_RESET);
}

void clearDIR(void) {
	HAL_GPIO_WritePin(SRCLR_DIR_GPIO_Port, SRCLR_DIR_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(SRCLR_EN_GPIO_Port, SRCLR_EN_Pin, GPIO_PIN_SET);

	HAL_GPIO_WritePin(RCLK_DIR_GPIO_Port, RCLK_DIR_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(RCLK_DIR_GPIO_Port, RCLK_DIR_Pin, GPIO_PIN_RESET);
}
