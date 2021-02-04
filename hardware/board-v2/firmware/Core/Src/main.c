/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "relays.h"
#include "gm009605.h"
#include "fonts.h"
#include "buttons.h"
#include "cmsis_os.h"
#include "horse_anim.h"


I2C_HandleTypeDef hi2c1;
int direction = 1;

char * cmd_buf;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;

// Global vars here for an unknown reason.
// It feels like when they are here the performance increases.
gpio_entity keys;
buttons btns;
uint8_t btns_clicked[BUTTONS_COUNT];

/* Definitions for command_reader */
osThreadId_t command_readerHandle;
const osThreadAttr_t command_reader_attributes = {
  .name = "command_reader",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 256 * 4
};
/* Definitions for user_interface */
osThreadId_t user_interfaceHandle;
const osThreadAttr_t user_interface_attributes = {
  .name = "user_interface",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 128 * 4
};
/* Definitions for keys_switcher */
osThreadId_t keys_switcherHandle;
const osThreadAttr_t keys_switcher_attributes = {
  .name = "keys_switcher",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 128 * 4
};
/* Definitions for rs485_control */
osThreadId_t rs485_controlHandle;
const osThreadAttr_t rs485_control_attributes = {
  .name = "rs485_control",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 128 * 4
};
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
//static void MX_USART3_UART_Init(void);
void Start_command_reader(void *argument);
void Start_user_interface(void *argument);
void Start_keys_switcher(void *argument);
void Start_rs485_controller(void *argument);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void) {
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  //MX_USART3_UART_Init();
  osKernelInitialize();
  init_gpio_entity(&keys);
  init_buttons(&btns);
  cmd_buf = init_bufferptr();
  init_sendbufferptr();
  SSD1306_Init();
  command_readerHandle = osThreadNew(Start_command_reader, NULL, &command_reader_attributes);
  user_interfaceHandle = osThreadNew(Start_user_interface, NULL, &user_interface_attributes);

  /* creation of keys_switcher */
  keys_switcherHandle = osThreadNew(Start_keys_switcher, NULL, &keys_switcher_attributes);

  /* creation of rs485_control */
  rs485_controlHandle = osThreadNew(Start_rs485_controller, NULL, &rs485_control_attributes);

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
	  osDelay(1000);

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_I2C1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK1;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_HSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void) {
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x0000020B;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK) {
    Error_Handler();
  }
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK) {
    Error_Handler();
  }
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK) {
    Error_Handler();
  }
}

static void MX_USART1_UART_Init(void) {
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 38400;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_RS485Ex_Init(&huart1, UART_DE_POLARITY_HIGH, 0, 0) != HAL_OK) {
    Error_Handler();
  }

  USART1->CR1 |= USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE;
 // USART1->CR1 &= ~USART_CR1_TXEIE;
  //| USART_CR1_TXEIE;
  USART1->CR2 = 0;
  USART1->CR3 = 0;
  NVIC_EnableIRQ(USART1_IRQn);
}

static void MX_USART2_UART_Init(void) {
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 38400;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_RS485Ex_Init(&huart2, UART_DE_POLARITY_HIGH, 0, 0) != HAL_OK) {
    Error_Handler();
  }
  USART2->CR1 |= USART_CR1_TE | USART_CR1_RE;
  //| USART_CR1_RXNEIE;
  USART2->CR2 = 0;
  USART2->CR3 = 0;
  //NVIC_EnableIRQ(USART2_IRQn);
}

static void MX_GPIO_Init(void) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, OUT_11_Pin|OUT_10_Pin|OUT_9_Pin|OUT_8_Pin
                          |OUT_7_Pin|OUT_1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, OUT_6_Pin|OUT_5_Pin|OUT_4_Pin|OUT_3_Pin
                          |OUT_2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : BUTTON_1_Pin BUTTON_2_Pin BUTTON_3_Pin */
  GPIO_InitStruct.Pin = BUTTON_1_Pin|BUTTON_2_Pin|BUTTON_3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : OUT_11_Pin OUT_10_Pin OUT_9_Pin OUT_8_Pin
                           OUT_7_Pin OUT_1_Pin */
  GPIO_InitStruct.Pin = OUT_11_Pin|OUT_10_Pin|OUT_9_Pin|OUT_8_Pin
                          |OUT_7_Pin|OUT_1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : OUT_6_Pin OUT_5_Pin OUT_4_Pin OUT_3_Pin
                           OUT_2_Pin */
  GPIO_InitStruct.Pin = OUT_6_Pin|OUT_5_Pin|OUT_4_Pin|OUT_3_Pin
                          |OUT_2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

void Start_command_reader(void *argument) {
  for(;;) {
	  osDelay(1000);
  }
}

void display_horse(int key) {
  SSD1306_Clear();
  SSD1306_GotoXY (0, 44);
  SSD1306_Mirror(1, 1);
  SSD1306_Puts (cmd_buf, &Font_7x10, 1);
  SSD1306_UpdateScreen();
}

void Start_user_interface(void *argument)
{
  /* USER CODE BEGIN Start_user_interface */
  /* Infinite loop */
  SSD1306_Clear();
  SSD1306_GotoXY(0,0);
  SSD1306_Mirror(1, 1);
  SSD1306_Puts ("openrbt.com", &Font_11x18, 1);
  SSD1306_GotoXY(12,30);
  SSD1306_Puts ("WELCOME :)", &Font_11x18, 1);
  SSD1306_UpdateScreen();
  SSD1306_ON();
  osDelay(2000);
  for(;;)
  {
	if(is_clicked(0)) {
	  display_horse(1);
	  send_bytes("0123456789\n", 11);
	}
	display_horse(1);

    osDelay(10);

  }
}

void Start_keys_switcher(void *argument) {
  int frame = 0;
  int cursor = 1;
  for(;;) {
	frame++;
	if(frame>20) {
		turn_off(cursor);
		cursor = 1-cursor;
		turn_on(cursor);
		frame = 0;
	}
	turn_on(3);
	buttons_update();
    osDelay(5);
  }
}

void Start_rs485_controller(void *argument) {
  for(;;) {
    osDelay(1);
  }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
  if (htim->Instance == TIM6) {
    HAL_IncTick();
  }
}

void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

void turn_on(uint8_t key) {
	HAL_GPIO_WritePin(keys.relays[key].port, keys.relays[key].pin , GPIO_PIN_SET);
}

void turn_off(uint8_t key) {
	HAL_GPIO_WritePin(keys.relays[key].port, keys.relays[key].pin , GPIO_PIN_RESET);
}

void buttons_update() {
	for(uint8_t i=0;i<BUTTONS_COUNT;i++) {
		update_button(&btns.btn[i]);
	}
}

int is_clicked(uint8_t key) {
  if(btns.btn[key].is_clicked) {
	  btns.btn[key].is_clicked = 0;
	  return 1;
  }
  return 0;
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
