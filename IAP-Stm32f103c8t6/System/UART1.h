#ifndef __UART1_H
#define __UART1_H

#include "stm32f10x.h"                  // Device header
extern char Receive[4];

void Serial1_Init(uint32_t Baudrate);
void Serial_SendByte(uint8_t Byte);
void Serial_SendDataPacket(char* Packet);
void Serial_SendArray(uint8_t* Array,uint8_t Len);
void Serial_SendString(char* String);
uint8_t Serial_GetRxFlag(void);
void Serial1_SendByte(uint8_t Byte);

#endif
