#include "stm32f10x.h"                  // Device header
#include "freertos.h"
#include "queue.h"
#include "typedef.h"
#include "driver_as608.h"

QueueHandle_t xQueueUart2;
uint8_t	Packet[AS608_MAX_PACKET_SIZE];        
UartReceiver Uart2Receiver;
UartReceiver* Uart2_Rx = &Uart2Receiver;

/**
  * @brief  串口2发送一个字节
  * @param  Byte 要发送的字节
  * @retval 无
  */
void Serial2_SendBByte(uint8_t Byte)
{
	while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
	USART_SendData(USART2,Byte);
	while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);
}

/**
  * @brief  串口2发送一个数组
  * @param  Aarry 要发送的数组
  * @param  Length 数组长度
  * @retval 无
  */
void Serial2_SendAarry(uint8_t* Aarry ,uint8_t Length)
{
	for(uint8_t i = 0;i < Length; i++)
	{
		Serial2_SendBByte(Aarry[i]);
	}
}

/**
  * @brief  串口2接收初始化
  * @param  无
  * @retval 无
  */
void Uart2_Receiver_Init(void) {
    Uart2_Rx->state = STATE_WAIT_HEADER1;
    Uart2_Rx->buffer = Packet;
    Uart2_Rx->expected_len = 0;
    Uart2_Rx->received_len = 0;
}

/**
  * @brief  串口2初始化
  * @param  无
  * @retval 无
  */
void Uart2_Init(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);//PA2TX引脚初始化
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);//PA3RX引脚初始化
    
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate=57600;    //串口波特率
    USART_InitStructure.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode=USART_Mode_Tx| USART_Mode_Rx;
    USART_InitStructure.USART_Parity=USART_Parity_No;
    USART_InitStructure.USART_StopBits=USART_StopBits_1;    //停止位
    USART_InitStructure.USART_WordLength=USART_WordLength_8b;//数据位
    USART_Init(USART2,&USART_InitStructure);
    
    USART_ITConfig(USART2,USART_IT_RXNE,ENABLE);
    
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel=USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority=1;
    NVIC_Init(&NVIC_InitStructure);
    
    USART_Cmd(USART2,ENABLE);
}

/**
  * @brief  处理接收到的字节
  * @param  byte 接收到的字节
  * @retval 无
  */
void vProcessReceivedByte(uint8_t byte) {
	
    if (Uart2_Rx->data_len + 2 + 4 + 3 > AS608_MAX_PACKET_SIZE) {
        // 数据包长度超过缓冲区大小，重新初始化接收状态机
        Uart2_Receiver_Init();
        return;
    }
	
	switch(Uart2_Rx -> state){
		case STATE_WAIT_HEADER1:
			Uart2_Rx->buffer[Uart2_Rx ->received_len++] = byte;
			if(Uart2_Rx-> received_len == 2){
				uint16_t data = ((uint16_t)Uart2_Rx->buffer[Uart2_Rx->received_len - 2] << 8 )|
					(uint16_t)Uart2_Rx->buffer[Uart2_Rx->received_len - 1] ;
				
				if(data == AS608_HEADER){
					Uart2_Rx->received_len = 2;
					Uart2_Rx->state = STATE_WAIT_HEADER2;
				}else{
					Uart2_Rx->received_len = 0;
				}
			}
			break;
		case STATE_WAIT_HEADER2:
			Uart2_Rx->buffer[Uart2_Rx ->received_len++] = byte;
			if( Uart2_Rx->received_len == 6){
				
				uint32_t data = ((uint32_t)Uart2_Rx->buffer[Uart2_Rx->received_len - 4] << 24) |
					((uint32_t)Uart2_Rx->buffer[Uart2_Rx->received_len - 3] << 16) |
					((uint32_t)Uart2_Rx->buffer[Uart2_Rx->received_len - 2] << 8)  |
					(uint32_t)Uart2_Rx->buffer[Uart2_Rx->received_len - 1] ;
				
				if(data == AS608_ADDRESS)
					Uart2_Rx->state = STATE_READ_LENGTH;
				else{
					Uart2_Receiver_Init();
				}
				
			}							
			break;
		case STATE_READ_LENGTH:
			Uart2_Rx->buffer[Uart2_Rx ->received_len++] = byte;
			if( Uart2_Rx->received_len == 9){
				Uart2_Rx -> data_len = ((uint16_t)Uart2_Rx->buffer[Uart2_Rx->received_len - 2] << 8) |
					(uint16_t)Uart2_Rx->buffer[Uart2_Rx->received_len - 1];
				Uart2_Rx->expected_len = 2 + 4 + 3 + Uart2_Rx->data_len;
				Uart2_Rx->state = STATE_READ_DATA;
			}	
			break;			
		case STATE_READ_DATA:
			Uart2_Rx->buffer[Uart2_Rx->received_len++] = byte;
			if (Uart2_Rx->received_len == Uart2_Rx->expected_len - 2) {
				 Uart2_Rx->state = STATE_READ_CheckSum;
			}
			break;
		case STATE_READ_CheckSum:
			//校验位计算时间太长不宜在中断执行，交给应用层管理
			Uart2_Rx->buffer[Uart2_Rx->received_len++] = byte;
			if (Uart2_Rx->received_len == Uart2_Rx->expected_len ){
				 BaseType_t xHigherPriorityTaskWoken = pdFALSE; 
				 if(xQueueSendFromISR(xQueueUart2,Packet, &xHigherPriorityTaskWoken) != pdPASS){
					//队列发送失败
				 }
				 portYIELD_FROM_ISR(xHigherPriorityTaskWoken);	 //触发任务转换
				 Uart2_Receiver_Init();
			}
			break;
	}
}
/**
  * @brief  串口2初始化
  * @param  无
  * @retval 无
  */
void Serial2_Init(void){
    // 在函数内部创建队列
    xQueueUart2 = xQueueCreate(3, sizeof(Packet));
    if (xQueueUart2 == NULL) {
        // 队列创建失败，可添加错误处理代码
    }
	 Uart2_Init();
	 Uart2_Receiver_Init();
}

/**
  * @brief  串口2中断处理函数
  * @param  无
  * @retval 无
  */
void USART2_IRQHandler(void)
{
	uint8_t byte;
	if( USART_GetITStatus(USART2,USART_IT_RXNE) == SET){
		byte = USART_ReceiveData(USART2);
		vProcessReceivedByte(byte);		
		USART_ClearITPendingBit(USART2,USART_IT_RXNE); // 清除接收中断标志位
	}
	
}

