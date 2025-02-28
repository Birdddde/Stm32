#include "stm32f10x.h"                  // Device header

#ifndef __UART1_H
#define __UART1_H

#define RX_BUFFER_SIZE 128

typedef struct Uart_RX{
	uint8_t* rx_buffer;      // DMA直接操作的缓冲区
	volatile uint16_t rx_data_length;
}Uart_Rx_t;

void USART1_Init(uint32_t baudrate);
void DMA1_Init(void);
uint8_t USART1_GetRxData(Uart_Rx_t* uart_Rx);
void Serial1_SendString(char* String);


#endif
