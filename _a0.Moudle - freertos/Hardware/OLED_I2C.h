#ifndef __OLED_I2C_H
#define __OLED_I2C_H

void OLED_I2C_Init(void);
void OLED_I2C_Start(void);
void OLED_I2C_Stop(void);
void OLED_I2C_SendByte(uint8_t Byte);
uint8_t OLED_I2C_R_SDA(void);
uint8_t OLED_I2C_ReceiveACK(void);

#endif
