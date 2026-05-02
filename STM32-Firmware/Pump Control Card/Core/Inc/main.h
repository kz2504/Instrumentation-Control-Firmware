/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_Pin GPIO_PIN_13
#define LED_GPIO_Port GPIOC
#define SRCLR_DIR_Pin GPIO_PIN_0
#define SRCLR_DIR_GPIO_Port GPIOA
#define RCLK_DIR_Pin GPIO_PIN_1
#define RCLK_DIR_GPIO_Port GPIOA
#define IRQ_Pin GPIO_PIN_2
#define IRQ_GPIO_Port GPIOA
#define EN_CS_Pin GPIO_PIN_3
#define EN_CS_GPIO_Port GPIOA
#define RST_Pin GPIO_PIN_0
#define RST_GPIO_Port GPIOB
#define SYNC_Pin GPIO_PIN_1
#define SYNC_GPIO_Port GPIOB
#define A0_CS_Pin GPIO_PIN_10
#define A0_CS_GPIO_Port GPIOB
#define A1_CS_Pin GPIO_PIN_11
#define A1_CS_GPIO_Port GPIOB
#define A2_CS_Pin GPIO_PIN_12
#define A2_CS_GPIO_Port GPIOB
#define DIN_DAC_Pin GPIO_PIN_13
#define DIN_DAC_GPIO_Port GPIOB
#define SRCLK_DIR_Pin GPIO_PIN_14
#define SRCLK_DIR_GPIO_Port GPIOB
#define SCLK_DAC_Pin GPIO_PIN_15
#define SCLK_DAC_GPIO_Port GPIOB
#define SER_DIR_Pin GPIO_PIN_9
#define SER_DIR_GPIO_Port GPIOA
#define SER_EN_Pin GPIO_PIN_4
#define SER_EN_GPIO_Port GPIOB
#define RCLK_EN_Pin GPIO_PIN_5
#define RCLK_EN_GPIO_Port GPIOB
#define SRCLK_EN_Pin GPIO_PIN_6
#define SRCLK_EN_GPIO_Port GPIOB
#define SRCLR_EN_Pin GPIO_PIN_7
#define SRCLR_EN_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
