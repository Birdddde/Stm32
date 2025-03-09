#include "stm32f10x.h"                  // Device header
#include "Delay.h" 

#define PIN_SCL GPIO_Pin_6
#define PIN_SDA GPIO_Pin_7
#define I2C_W_SCL(x) GPIO_WriteBit(GPIOB,GPIO_Pin_6,(BitAction)(x))
#define I2C_W_SDA(x) GPIO_WriteBit(GPIOB,GPIO_Pin_7,(BitAction)(x))

void OLED_I2C_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = PIN_SCL;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = PIN_SDA;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	I2C_W_SCL(1);
	I2C_W_SDA(1);
}

void OLED_I2C_Start(void)
{
	I2C_W_SDA(1);
	I2C_W_SCL(1);
	I2C_W_SDA(0);
	I2C_W_SCL(0);
}

void OLED_I2C_Stop(void)
{
	I2C_W_SDA(0);
	I2C_W_SCL(1);
	I2C_W_SDA(1);
}
void OLED_I2C_SendByte(uint8_t Byte)
{
	uint8_t i;
	for( i=0;i<8;i++)
	{
		I2C_W_SDA( (Byte &(0x80>>i)) );
		I2C_W_SCL(1);
		I2C_W_SCL(0);
	}
}


uint8_t OLED_I2C_R_SDA(void)
{
	uint8_t BitValue;
	BitValue=GPIO_ReadInputDataBit(GPIOB,PIN_SDA);
	Delay_us(10);
	return BitValue;
}

uint8_t OLED_I2C_ReceiveACK(void)
{
	I2C_W_SDA(1);
	I2C_W_SCL(1);
	uint8_t ACKBit=OLED_I2C_R_SDA();
	I2C_W_SCL(0);
	return ACKBit;
}


