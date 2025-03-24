#include "stm32f10x.h"                  // Device header

typedef enum {
    WAIT_START,     // 等待开始标志 '@'
    RECEIVING_DATA, // 接收数据
    WAIT_END        // 等待结束标志 '$'
} PacketState;

// 数据包缓冲区
#define MAX_PACKET_SIZE 128
static uint8_t uart3_rx_buff[MAX_PACKET_SIZE];
static uint16_t packetIndex = 0;
static PacketState currentState = WAIT_START;
static uint8_t Flag_rx_fin = 0;
	
void Serial3_SendBByte(uint8_t Byte)
{
	while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
	USART_SendData(USART3,Byte);
	while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET);
}

void Serial3_SendAarry(uint8_t* Aarry ,uint8_t Length)
{
	for(uint8_t i = 0;i < Length; i++)
	{
		Serial3_SendBByte(Aarry[i]);
	}
}

void Serial3_SendString(char* String)
{
    char* pS=String;
    while(*pS!='\0')
    {  
        while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
        USART_SendData(USART3,*pS);
        while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET);
        pS++;
    }
}

uint8_t* Uart3_GetData(uint8_t * Flag){
	if(Flag_rx_fin == 1)
	{
		*Flag = Flag_rx_fin;
		Flag_rx_fin = 0;
	}
	return uart3_rx_buff;
}

void Serial3_Init(uint32_t Baudrate)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);//PB10TX引脚初始化
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);//PB11RX引脚初始化
    
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate=Baudrate;    //串口波特率
    USART_InitStructure.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode=USART_Mode_Tx| USART_Mode_Rx;
    USART_InitStructure.USART_Parity=USART_Parity_No;
    USART_InitStructure.USART_StopBits=USART_StopBits_1;    //停止位
    USART_InitStructure.USART_WordLength=USART_WordLength_8b;//数据位
    USART_Init(USART3,&USART_InitStructure);
    
    USART_ITConfig(USART3,USART_IT_RXNE,ENABLE);
    
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel=USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority=1;
    NVIC_Init(&NVIC_InitStructure);
    
    USART_Cmd(USART3,ENABLE);
}

static void vProcessReceivedByte(uint8_t byte) {
    switch(currentState) {
        case WAIT_START:
			   Flag_rx_fin = 0;
            if(byte == '@') {
                currentState = RECEIVING_DATA;
                packetIndex = 0;
            }
            break;
            
        case RECEIVING_DATA:
            if(byte == '$') {
                currentState = WAIT_END;
                if(packetIndex > 0) {                   
                    Flag_rx_fin = 1;// 数据包接收完成，设置标志位
                }else {                   
                    currentState = WAIT_START;// 空数据包，重置状态
                    Flag_rx_fin = 0;
                }
                currentState = WAIT_START;
            } else if(packetIndex < MAX_PACKET_SIZE) {
                uart3_rx_buff[packetIndex++] = byte;
            }else {
                // 缓冲区溢出，重置状态
                currentState = WAIT_START;
                Flag_rx_fin = 0;
                packetIndex = 0;
            }
            break;
            
        case WAIT_END:
            currentState = WAIT_START;
            break;
    }
}

// 串口接收中断服务函数
void USART3_IRQHandler(void)
{
	uint8_t byte;
	if( USART_GetITStatus(USART3,USART_IT_RXNE) == SET){
		byte = USART_ReceiveData(USART3);
		vProcessReceivedByte(byte);		
		USART_ClearITPendingBit(USART3,USART_IT_RXNE); // 清除接收中断标志位
	}
	
}

