#include "stm32f10x.h"                  // Device header
#include "driver_as608.h"
#include <math.h>
#include "uart2.h"
#include "typedef.h"

#include "freertos.h"
#include "queue.h"

#define CHECKSUM_BYTES 2
#define CONFIRMCODE_BYTES 1
#define Command_BYTES 1

extern QueueHandle_t xQueueUart2; 
uint8_t buffer[AS608_MAX_PACKET_SIZE];
As608_PacketInfo_t As608_Packet_t={0xEF01,0xFFFFFFFF};
As608_PacketInfo_t* As608_Packet = &As608_Packet_t;

uint8_t AS608_Read(void){

	uint8_t pindex = 10;
	uint16_t sum = 0;
	if(xQueueReceive(xQueueUart2,buffer, pdMS_TO_TICKS(100) ) == pdPASS){		

		As608_Packet->Identifier = buffer[6];
		As608_Packet->Length = (uint16_t)buffer[7] << 8 | (uint16_t)buffer[8];
		As608_Packet->ConfirmCode = buffer[9];
		
		for(uint16_t i = 0; i < As608_Packet->Length - CHECKSUM_BYTES - CONFIRMCODE_BYTES; i++)
		{
			As608_Packet->Params[i] = buffer[pindex++];
			sum += As608_Packet->Params[i];
		}
		
		sum += As608_Packet->Identifier + As608_Packet->Length + As608_Packet->ConfirmCode;
		As608_Packet->CheckSum = (uint16_t)buffer[pindex] << 8|
			(uint16_t)buffer[pindex+1];
		if(As608_Packet->CheckSum == sum){
			return 1;
		}
		
	}
	return 0;
}

uint8_t AS608_SendCommand(uint8_t Identifier,uint16_t Packet_Length,uint8_t Command,uint8_t* Params){
	
	uint16_t checknum = Identifier + Packet_Length + Command;
	
	Serial2_SendBByte(AS608_HEADER >> 8);					//包头
	Serial2_SendBByte((uint8_t)AS608_HEADER);	
	
	Serial2_SendBByte( (uint8_t)(AS608_ADDRESS >> 24) );	//芯片地址
	Serial2_SendBByte( (uint8_t)(AS608_ADDRESS >> 16) );
	Serial2_SendBByte( (uint8_t)(AS608_ADDRESS >> 8) );
	Serial2_SendBByte( (uint8_t) AS608_ADDRESS  );
	
	Serial2_SendBByte(Identifier);							//包标识
	Serial2_SendBByte(Packet_Length >> 8);
	
	Serial2_SendBByte(Packet_Length);						//包长度
	
	Serial2_SendBByte(Command);								//指令码
	
	for(uint16_t i = 0 ; i < Packet_Length - CHECKSUM_BYTES - Command_BYTES ; i++)
	{
		Serial2_SendBByte(Params[i]);
		checknum += Params[i];
	}
	Serial2_SendBByte(checknum >> 8);								//校验和
	Serial2_SendBByte(checknum);
	
	return 0;
}

void PS_StaGPIO_Init(void)
{   
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOB,GPIO_Pin_12);
	
}

// 系统初始化函数
void AS608_Init(void) {
	 Serial2_Init();
	 PS_StaGPIO_Init();
}


uint8_t PS_GetImage(as608_status_t* status ){

	AS608_SendCommand(AS608_TYPE_COMMAND,0x0003,AS608_COMMAND_GET_IMAGE,NULL);
	vTaskDelay(pdMS_TO_TICKS(500));
	if(AS608_Read()){
		*status =(as608_status_t)As608_Packet->ConfirmCode;
		return 1;	//Receive sucessed
	}
		return 0;	//Receive failed
}

