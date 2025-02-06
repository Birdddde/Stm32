#ifndef __USART_H
#define __USART_H

#include "stm32f10x.h"                  // Device header

void Serial_Init(void);
void Serial_SendByte(uint8_t Byte);
void Serial_SendDataPacket(char* Packet);
void Serial_SendArray(uint8_t* Array,uint8_t Len);
void Serial_SendString(char* String);
uint8_t Serial_GetRxFlag(void);
extern char Receive[4];

#endif
