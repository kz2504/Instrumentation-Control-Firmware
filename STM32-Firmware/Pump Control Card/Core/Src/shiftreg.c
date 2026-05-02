#include "shiftreg.h"

#include "main.h"

static uint8_t reverse8(uint8_t x)
{
  x = (uint8_t)(((x & 0xF0U) >> 4) | ((x & 0x0FU) << 4));
  x = (uint8_t)(((x & 0xCCU) >> 2) | ((x & 0x33U) << 2));
  x = (uint8_t)(((x & 0xAAU) >> 1) | ((x & 0x55U) << 1));
  return x;
}

void shiftByteEN(uint8_t byte)
{
  uint8_t bit_index;

  byte = reverse8(byte);

  HAL_GPIO_WritePin(SRCLR_EN_GPIO_Port, SRCLR_EN_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(RCLK_EN_GPIO_Port, RCLK_EN_Pin, GPIO_PIN_RESET);

  for (bit_index = 0U; bit_index < 8U; bit_index++)
  {
    HAL_GPIO_WritePin(SER_EN_GPIO_Port,
                      SER_EN_Pin,
                      (byte & 0x80U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(SRCLK_EN_GPIO_Port, SRCLK_EN_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(SRCLK_EN_GPIO_Port, SRCLK_EN_Pin, GPIO_PIN_RESET);
    byte = (uint8_t)(byte << 1);
  }

  HAL_GPIO_WritePin(RCLK_EN_GPIO_Port, RCLK_EN_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(RCLK_EN_GPIO_Port, RCLK_EN_Pin, GPIO_PIN_RESET);

  HAL_GPIO_WritePin(SER_EN_GPIO_Port, SER_EN_Pin, GPIO_PIN_RESET);
}

void shiftByteDIR(uint8_t byte)
{
  uint8_t bit_index;

  byte = reverse8(byte);

  HAL_GPIO_WritePin(SRCLR_DIR_GPIO_Port, SRCLR_DIR_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(RCLK_DIR_GPIO_Port, RCLK_DIR_Pin, GPIO_PIN_RESET);

  for (bit_index = 0U; bit_index < 8U; bit_index++)
  {
    HAL_GPIO_WritePin(SER_DIR_GPIO_Port,
                      SER_DIR_Pin,
                      (byte & 0x80U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(SRCLK_DIR_GPIO_Port, SRCLK_DIR_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(SRCLK_DIR_GPIO_Port, SRCLK_DIR_Pin, GPIO_PIN_RESET);
    byte = (uint8_t)(byte << 1);
  }

  HAL_GPIO_WritePin(RCLK_DIR_GPIO_Port, RCLK_DIR_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(RCLK_DIR_GPIO_Port, RCLK_DIR_Pin, GPIO_PIN_RESET);

  HAL_GPIO_WritePin(SER_DIR_GPIO_Port, SER_DIR_Pin, GPIO_PIN_RESET);
}

void clearEN(void)
{
  HAL_GPIO_WritePin(SRCLR_EN_GPIO_Port, SRCLR_EN_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(SRCLR_EN_GPIO_Port, SRCLR_EN_Pin, GPIO_PIN_SET);

  HAL_GPIO_WritePin(RCLK_EN_GPIO_Port, RCLK_EN_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(RCLK_EN_GPIO_Port, RCLK_EN_Pin, GPIO_PIN_RESET);
}

void clearDIR(void)
{
  HAL_GPIO_WritePin(SRCLR_DIR_GPIO_Port, SRCLR_DIR_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(SRCLR_DIR_GPIO_Port, SRCLR_DIR_Pin, GPIO_PIN_SET);

  HAL_GPIO_WritePin(RCLK_DIR_GPIO_Port, RCLK_DIR_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(RCLK_DIR_GPIO_Port, RCLK_DIR_Pin, GPIO_PIN_RESET);
}
