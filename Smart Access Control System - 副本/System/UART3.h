#ifndef __UART3_H
#define __UART3_H

void Serial3_Init(uint32_t Baudrate);
void Serial3_SendAarry(uint8_t* Aarry ,uint8_t Length);
void Serial3_SendBByte(uint8_t Byte);
void Serial3_SendString(char* String);
uint8_t* Uart3_GetData(uint8_t * Flag);

#endif
