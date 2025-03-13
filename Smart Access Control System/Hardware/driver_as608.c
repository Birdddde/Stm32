#include "stm32f10x.h"                  // Device header
#include "driver_as608.h"
#include "uart2.h"
#include "delay.h"

#include "freertos.h"
#include "queue.h"

#define CHECKSUM_BYTES 2
#define CONFIRMCODE_BYTES 1
#define Command_BYTES 1

extern QueueHandle_t xQueueUart2; 
uint8_t buffer[AS608_MAX_PACKET_SIZE];

As608_PacketInfo_t As608_Packet_t={0xEF01,0xFFFFFFFF};
As608_PacketInfo_t* As608_Packet = &As608_Packet_t;

void PS_StaGPIO_Init(void){   
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOA,GPIO_Pin_1);
}



void AS608_Init(void) {
	 Serial2_Init();
	 PS_StaGPIO_Init();
}

uint8_t AS608_Read(uint16_t delay_ms){
	
	uint8_t pindex = 10;
	uint16_t sum = 0;
	
	Delay_ms(delay_ms);	//等待AS608应答,延时函数

	if(xQueueReceive(xQueueUart2,buffer, pdMS_TO_TICKS(500) ) == pdPASS){		

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

uint8_t PS_GetImage(as608_status_t* Status){
	uint16_t buffer_length = 0x0003;	
	
	AS608_SendCommand(AS608_TYPE_COMMAND,buffer_length,AS608_COMMAND_GET_IMAGE,NULL);

	if(AS608_Read(1500)){
		*Status =(as608_status_t)As608_Packet->ConfirmCode;
		return 1;	//Receive sucessed
	}
		return 0;	//Receive failed
}

uint8_t PS_GenChar(as608_status_t* Status,as608_buffer_number_t Buffer_id){

	uint16_t buffer_length = 0x0004;
	
	uint8_t* buffer = (uint8_t*)pvPortMalloc(buffer_length - CHECKSUM_BYTES - Command_BYTES);
	if (buffer == NULL) return 2;		//memory malloc failed
	buffer[0] = Buffer_id;
	
	AS608_SendCommand(AS608_TYPE_COMMAND,0x0004,AS608_COMMAND_GEN_CHAR,buffer);
	vPortFree(buffer);
	
	if(AS608_Read(1500)){
		*Status =(as608_status_t)As608_Packet->ConfirmCode;
		return 1;	//Receive sucessed
	}
		return 0;	//Receive failed
}

uint8_t PS_Search(as608_status_t* Status,uint16_t* Page_id ,uint16_t* Match_score ,as608_buffer_number_t Buffer_id,uint16_t StartPage,uint16_t PageNum){

	uint16_t buffer_length = 0x0008;
	uint8_t* buffer = (uint8_t*)pvPortMalloc(buffer_length - CHECKSUM_BYTES - Command_BYTES);
	if (buffer == NULL) return 2;		//memory malloc failed
	
	buffer[0] = Buffer_id;
	buffer[1] = StartPage >> 8;
	buffer[2] = StartPage;
	buffer[3] = PageNum   >> 8;
	buffer[4] = PageNum;

	AS608_SendCommand(AS608_TYPE_COMMAND,buffer_length,AS608_COMMAND_SEARCH,buffer);
	vPortFree(buffer);
	
	if(AS608_Read(2000)){
		*Status =(as608_status_t)As608_Packet->ConfirmCode;
		*Page_id =(uint16_t) As608_Packet->Params[0] << 8|
			(uint16_t) As608_Packet ->Params[1];
		*Match_score =(uint16_t) As608_Packet->Params[2] << 8|
			(uint16_t) As608_Packet ->Params[3];
		return 1;	//Receive sucessed
	}
		return 0;	//Receive failed
}

uint8_t PS_RegModel(as608_status_t* Status){
	uint16_t buffer_length = 0x0003;	
	
	AS608_SendCommand(AS608_TYPE_COMMAND,buffer_length,AS608_COMMAND_REG_MODEL,NULL);
	
	if(AS608_Read(2000)){
		*Status =(as608_status_t)As608_Packet->ConfirmCode;
		return 1;	//Receive sucessed
	}
		return 0;	//Receive failed
}

uint8_t PS_StoreChar(as608_status_t* Status,as608_buffer_number_t Buffer_id,uint16_t Page_id){

	uint16_t buffer_length = 0x0006;
	uint8_t* buffer = (uint8_t*)pvPortMalloc(buffer_length - CHECKSUM_BYTES - Command_BYTES);
	if (buffer == NULL) return 2;		//memory malloc failed
	
	buffer[0] = Buffer_id;
	buffer[1] = Page_id >> 8;
	buffer[2] = Page_id;

	AS608_SendCommand(AS608_TYPE_COMMAND,buffer_length,AS608_COMMAND_STORE_CHAR,buffer);
	vPortFree(buffer);
	
	if(AS608_Read(2000)){
		*Status =(as608_status_t)As608_Packet->ConfirmCode;
		return 1;	//Receive sucessed
	}
		return 0;	//Receive failed
}

uint8_t PS_DeletChar(as608_status_t* Status,uint16_t Page_id,uint16_t Count){

	uint16_t buffer_length = 0x0007;
	uint8_t* buffer = (uint8_t*)pvPortMalloc(buffer_length - CHECKSUM_BYTES - Command_BYTES);
	if (buffer == NULL) return 2;		//memory malloc failed
	
	buffer[0] = Page_id >> 8;
	buffer[1] = Page_id;
	buffer[2] = Count >> 8;
	buffer[3] = Count;
	
	AS608_SendCommand(AS608_TYPE_COMMAND,buffer_length,AS608_COMMAND_DELETE_CHAR,buffer);
	vPortFree(buffer);
	
	if(AS608_Read(2000)){
		*Status =(as608_status_t)As608_Packet->ConfirmCode;
		return 1;	//Receive sucessed
	}
		return 0;	//Receive failed
}

uint8_t PS_Empty(as608_status_t* Status){
	uint16_t buffer_length = 0x0003;	
	
	AS608_SendCommand(AS608_TYPE_COMMAND,buffer_length,AS608_COMMAND_EMPTY,NULL);
		
	if(AS608_Read(3000)){
		*Status =(as608_status_t)As608_Packet->ConfirmCode;
		return 1;	//Receive sucessed
	}
		return 0;	//Receive failed
}



