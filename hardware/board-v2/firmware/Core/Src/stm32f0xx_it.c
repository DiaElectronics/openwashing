/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f0xx_it.c
  * @brief   Interrupt Service Routines.
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
#include "stm32f0xx_it.h"
#define MAX_CMD_BUF 63
#define MAX_SEND_BUF 32
#define MAX_DELAY_MS 100

extern TIM_HandleTypeDef htim6;
uint8_t stm_cmd_cursor;
char stm_cmd_buffer[MAX_CMD_BUF + 1];

char send_cycle_buffer[MAX_SEND_BUF + 2];
uint8_t send_cursor;
uint8_t remaining_bytes;

// let's use this mutex for remaining bytes
volatile xSemaphoreHandle xMutex;

/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M0 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void) {
  while (1) {
  }
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void) {
  while (1) {
  }
}

void TIM6_IRQHandler(void) {
  HAL_TIM_IRQHandler(&htim6);
}

char * init_bufferptr() {
	stm_cmd_cursor = 0;
	stm_cmd_buffer[0] = 0;
	return stm_cmd_buffer;
}

void init_sendbufferptr() {
	send_cycle_buffer[0] = 0;
	send_cursor = 0;
	remaining_bytes = 0;
}

uint8_t send_bytes(char * new_data, uint8_t size) {
	usart1_sending_off();
	if (remaining_bytes > 0) {
		return 3; // let's not allow sending something while the previous operation is in progress
	}
	if (size > MAX_SEND_BUF) {
		return 1;
	}
	uint8_t write_cursor = send_cursor + 1;
	for(uint8_t i = 0; i<size; i++) {
		if(write_cursor>=MAX_SEND_BUF) write_cursor = write_cursor - MAX_SEND_BUF;
		if(write_cursor>=MAX_SEND_BUF) {
			return 2;
		}
		send_cycle_buffer[write_cursor] = new_data[i];
		remaining_bytes++;
		write_cursor++;
	}
	usart1_sending_on();
	return 0;
}

uint8_t _send_byte_from_buffer() {
	if(remaining_bytes == 0) {
		usart1_sending_off();
		return 1;
	}
	if ((USART1->ISR & USART_ISR_TXE)==0) return 2;
	send_cursor = send_cursor + 1;

	if(send_cursor >= MAX_SEND_BUF) send_cursor = 0;
	uint8_t to_be_sent = (uint8_t)send_cycle_buffer[send_cursor];
	remaining_bytes = remaining_bytes - 1;
	USART1->TDR = to_be_sent;
	return 0;
}

uint8_t is_acceptable(char k) {
	if (k>='0' && k <='9') return 1;
	if (k=='+' || k=='-' || k==';' || k=='.'|| k==',' || k==' '|| k=='!') return 1;
	if (k>='a' && k<='z') return 1;
	if (k>='A' && k<='Z') return 1;
	return 0;
}

void _byte_accepted() {
	uint8_t d = USART1->RDR;
	if (is_acceptable((char)d)) {
		if(stm_cmd_cursor>=MAX_CMD_BUF) {
			stm_cmd_cursor = 0;
		}
		stm_cmd_buffer[stm_cmd_cursor] = (char)d;
		stm_cmd_buffer[stm_cmd_cursor + 1] = 0;
		stm_cmd_cursor = stm_cmd_cursor + 1;
	}
}

void usart1_sending_on() {
	USART1->CR1 |= USART_CR1_TXEIE;
}

void usart1_sending_off() {
	USART1->CR1 &= ~USART_CR1_TXEIE;
}

void USART1_IRQHandler(void) {
	if(USART1->ISR & USART_ISR_RXNE) {
		_byte_accepted();
		return;
	}
	if(USART1->ISR & USART_ISR_ORE) {
		USART1->ICR = (UART_CLEAR_OREF | UART_CLEAR_NEF | UART_CLEAR_PEF | UART_CLEAR_FEF);
		USART1->CR1 &= ~USART_CR1_IDLEIE;
		return;
	}
	if(USART1->ISR & USART_ISR_TXE) {
		_send_byte_from_buffer();
		return;
	}

}
