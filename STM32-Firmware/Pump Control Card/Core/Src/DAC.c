#include "DAC.h"

#include "main.h"

void dac_setCS(uint8_t line)
{
  uint8_t a0;
  uint8_t a1;
  uint8_t a2;

  HAL_GPIO_WritePin(EN_CS_GPIO_Port, EN_CS_Pin, GPIO_PIN_SET);

  a0 = (uint8_t)((line >> 0) & 0x01U);
  a1 = (uint8_t)((line >> 1) & 0x01U);
  a2 = (uint8_t)((line >> 2) & 0x01U);

  HAL_GPIO_WritePin(A0_CS_GPIO_Port, A0_CS_Pin, a0 ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(A1_CS_GPIO_Port, A1_CS_Pin, a1 ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(A2_CS_GPIO_Port, A2_CS_Pin, a2 ? GPIO_PIN_SET : GPIO_PIN_RESET);

  HAL_GPIO_WritePin(EN_CS_GPIO_Port, EN_CS_Pin, GPIO_PIN_RESET);
}

void dac_resetCS(void)
{
  HAL_GPIO_WritePin(EN_CS_GPIO_Port, EN_CS_Pin, GPIO_PIN_SET);
}

void writeDAC(uint8_t channel, float AVDD, float vOut, uint8_t mode)
{
  float code_f;
  uint16_t code_u16;
  uint16_t word;
  int8_t bit_index;

  code_f = (vOut / AVDD) * 4096.0f;
  if (code_f <= 0.0f)
  {
    code_f = 0.0f;
  }
  else if (code_f >= 4095.0f)
  {
    code_f = 4095.0f;
  }

  code_u16 = (uint16_t)code_f;
  word = (uint16_t)((((uint16_t)mode & 0x0003U) << 14) | ((code_u16 & 0x0FFFU) << 2));

  dac_setCS(channel);

  HAL_GPIO_WritePin(SCLK_DAC_GPIO_Port, SCLK_DAC_Pin, GPIO_PIN_RESET);

  for (bit_index = 15; bit_index >= 0; bit_index--)
  {
    HAL_GPIO_WritePin(DIN_DAC_GPIO_Port,
                      DIN_DAC_Pin,
                      (word & (uint16_t)(1U << bit_index)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(SCLK_DAC_GPIO_Port, SCLK_DAC_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(SCLK_DAC_GPIO_Port, SCLK_DAC_Pin, GPIO_PIN_RESET);
  }

  HAL_GPIO_WritePin(DIN_DAC_GPIO_Port, DIN_DAC_Pin, GPIO_PIN_RESET);
  dac_resetCS();
}
