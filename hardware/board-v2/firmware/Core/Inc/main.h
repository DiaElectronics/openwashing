/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
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
#include "stm32f0xx_hal.h"

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
#define BUTTON_1_Pin GPIO_PIN_13
#define BUTTON_1_GPIO_Port GPIOC
#define BUTTON_2_Pin GPIO_PIN_14
#define BUTTON_2_GPIO_Port GPIOC
#define BUTTON_3_Pin GPIO_PIN_15
#define BUTTON_3_GPIO_Port GPIOC
#define OUT_11_Pin GPIO_PIN_0
#define OUT_11_GPIO_Port GPIOA
#define OUT_10_Pin GPIO_PIN_4
#define OUT_10_GPIO_Port GPIOA
#define OUT_9_Pin GPIO_PIN_5
#define OUT_9_GPIO_Port GPIOA
#define OUT_8_Pin GPIO_PIN_6
#define OUT_8_GPIO_Port GPIOA
#define OUT_7_Pin GPIO_PIN_7
#define OUT_7_GPIO_Port GPIOA
#define OUT_6_Pin GPIO_PIN_0
#define OUT_6_GPIO_Port GPIOB
#define OUT_5_Pin GPIO_PIN_12
#define OUT_5_GPIO_Port GPIOB
#define OUT_4_Pin GPIO_PIN_13
#define OUT_4_GPIO_Port GPIOB
#define OUT_3_Pin GPIO_PIN_14
#define OUT_3_GPIO_Port GPIOB
#define OUT_2_Pin GPIO_PIN_15
#define OUT_2_GPIO_Port GPIOB
#define OUT_1_Pin GPIO_PIN_8
#define OUT_1_GPIO_Port GPIOA
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

void turn_on(uint8_t key);
void turn_off(uint8_t key);
void buttons_update();
int is_clicked(uint8_t key);
char * init_bufferptr();
void init_sendbufferptr();
uint8_t _send_byte_from_buffer();
uint8_t send_bytes(char * new_data, uint8_t size);
#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
