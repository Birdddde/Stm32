#include "stm32f10x.h"                  // Device header
#include "string.h"
#include "stdio.h"

/* notice : due to the modification of the printf() function,You must 
    Enable the option(option for Target):use MicroLib
*/

#define RECEIVE_BUFFER_SIZE 50

uint8_t g_Serial_RxFlag = 0; // 接收标志位
uint8_t pRxHead = 0;          // 环形缓冲区读指针
uint8_t pRxTail = 0;          // 环形缓冲区写指针
uint8_t Receive[RECEIVE_BUFFER_SIZE]; // 接收缓冲区

// 计算环形缓冲区中数据的数量
uint8_t Serial1_GetRxCount(void) {
    return (pRxTail - pRxHead + RECEIVE_BUFFER_SIZE) % RECEIVE_BUFFER_SIZE;
}

void Serial1_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);
     
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
    USART_InitStructure.USART_BaudRate=115200;    //串口波特率
    USART_InitStructure.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode=USART_Mode_Tx| USART_Mode_Rx;
    USART_InitStructure.USART_Parity=USART_Parity_No;
    USART_InitStructure.USART_StopBits=USART_StopBits_1;    //停止位
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

void Serial1_SendString(char* String)
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

// 获取接收标志位并清空缓冲区
uint8_t Serial1_GetRxFlag(void)
{
    if(g_Serial_RxFlag==1)
    {
        g_Serial_RxFlag=0;
        pRxHead = 0;
        pRxTail = 0;
        memset(&Receive, 0, sizeof(Receive));    // 清空接收缓冲区
        return 1;
    }
    return 0;
}

// 从环形缓冲区读取数据
uint8_t Serial1_ReadData(uint8_t *pData) {
    if (pRxHead != pRxTail) {
        *pData = Receive[pRxHead];
        pRxHead = (pRxHead + 1) % RECEIVE_BUFFER_SIZE;
        return 1;
    }
    return 0;
}

void USART1_IRQHandler(void)
{
    if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET)
    {
        uint8_t RxData = USART_ReceiveData(USART1);

        if (!g_Serial_RxFlag) {
            g_Serial_RxFlag = 1; // 置接收标志位
        }

        // 计算下一个写入位置
        uint8_t nextTail = (pRxTail + 1) % RECEIVE_BUFFER_SIZE;
        if (nextTail != pRxHead) { // 缓冲区未满
            Receive[pRxTail] = RxData; 
            pRxTail = nextTail;
        }

        USART_ClearITPendingBit(USART1, USART_IT_RXNE); // 清除接收中断标志位
    }
}

int fputc( int ch, FILE *f ){
        USART_SendData(USART1,(u8) ch );
        while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);
        return ch;
}
