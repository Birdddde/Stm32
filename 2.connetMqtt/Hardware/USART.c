#include "stm32f10x.h"                  // Device header
#include "OLED.h"
#include "string.h"

#define RECEIVE_BUFFER_SIZE 100

uint8_t g_Serial_RxFlag = 0; // 接收标志位
uint8_t pReceive = 0;        // 接收缓冲区指针
uint8_t Receive[RECEIVE_BUFFER_SIZE]; // 接收缓冲区

void Serial_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);//PA9TX引脚初始化
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);//PA10RX引脚初始化
	
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate=115200;	//串口波特率
	USART_InitStructure.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode=USART_Mode_Tx| USART_Mode_Rx;
	USART_InitStructure.USART_Parity=USART_Parity_No;
	USART_InitStructure.USART_StopBits=USART_StopBits_1;	//停止位
	USART_InitStructure.USART_WordLength=USART_WordLength_8b;//数据位
	USART_Init(USART1,&USART_InitStructure);
	
	USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel=USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=1;
	NVIC_Init(&NVIC_InitStructure);
	
	USART_Cmd(USART1,ENABLE);
}

void Serial_SendString(char* String)
{
	char* pS=String;
	while(*pS!='\0')
	{  
		while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
		USART_SendData(USART1,*pS);
		while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
		pS++;
	}
}

uint8_t Serial_GetRxFlag(void)
{
	if(g_Serial_RxFlag==1)
	{
		g_Serial_RxFlag=0;
		return 1;
	}
	pReceive=0;
	memset(&Receive, 0, sizeof(Receive));		// 清空接收缓冲区
	return 0;
}

void USART1_IRQHandler(void)
{

    if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET)
    {
        uint8_t RxData = USART_ReceiveData(USART1);

        if (!g_Serial_RxFlag)	{
			  g_Serial_RxFlag = 1; // 置接收标志位
		  }

        // 将接收到的数据存入缓冲区
        Receive[pReceive] = RxData;
        pReceive++;
        USART_ClearITPendingBit(USART1, USART_IT_RXNE); // 清除接收中断标志位
    }
}	



	
