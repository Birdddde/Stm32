#include "stm32f10x.h"
#include "string.h"
#include "stdio.h"

/* 注意：修改了printf()函数，必须在工程选项中启用MicroLib */

#define RECEIVE_BUFFER_SIZE 50

volatile uint8_t g_Serial_RxFlag = 0;    // 接收标志位
volatile uint8_t g_Serial_OverflowFlag = 0; // 溢出标志位
volatile uint8_t pRxHead = 0;           // 环形缓冲区读指针
volatile uint8_t pRxTail = 0;           // 环形缓冲区写指针
uint8_t Receive[RECEIVE_BUFFER_SIZE];    // 接收缓冲区

// 获取接收数据数量（带临界区保护）
uint8_t Serial1_GetRxCount(void) {
    uint8_t count;
    __disable_irq();
    count = (pRxTail - pRxHead + RECEIVE_BUFFER_SIZE) % RECEIVE_BUFFER_SIZE;
    __enable_irq();
    return count;
}

void Serial1_Init(uint32_t Baudrate) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);
    
    GPIO_InitTypeDef GPIO_InitStructure;
    // 配置TX引脚为复用推挽输出
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // 配置RX引脚为浮空输入
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = Baudrate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART1, &USART_InitStructure);
    
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    USART_Cmd(USART1, ENABLE);
}

void Serial1_SendString(char* String) {
    char* p = String;
    while (*p != '\0') {
        while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
        USART_SendData(USART1, *p++);
    }
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
}

void Serial1_SendByte(uint8_t Byte) {

	while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
	USART_SendData(USART1, Byte);
	while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
}

// 获取接收标志位（不修改状态）
uint8_t Serial1_GetRxFlag(void) {
    return g_Serial_RxFlag;
}

// 清除接收标志位和缓冲区
void Serial1_ClearRxFlag(void) {
    __disable_irq();
    g_Serial_RxFlag = 0;
    pRxHead = pRxTail; // 清空缓冲区
    __enable_irq();
}

// 读取数据（带临界区保护）
uint8_t Serial1_ReadData(uint8_t* pData) {
    uint8_t result = 0;
    __disable_irq();
    if (pRxHead != pRxTail) {
        *pData = Receive[pRxHead];
        pRxHead = (pRxHead + 1) % RECEIVE_BUFFER_SIZE;
        result = 1;
    }
    __enable_irq();
    return result;
}

// 获取溢出标志并清除
uint8_t Serial1_GetOverflowFlag(void) {
    uint8_t flag = g_Serial_OverflowFlag;
    g_Serial_OverflowFlag = 0;
    return flag;
}

// 中断服务函数
void USART1_IRQHandler(void) {
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
        uint8_t data = USART_ReceiveData(USART1);
        
        // 计算下一个写入位置
        uint8_t nextTail = (pRxTail + 1) % RECEIVE_BUFFER_SIZE;
        
        if (nextTail != pRxHead) { // 缓冲区未满
            Receive[pRxTail] = data;
            pRxTail = nextTail;
            g_Serial_RxFlag = 1; // 设置接收标志
        } else {
            g_Serial_OverflowFlag = 1; // 设置溢出标志
        }
        
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }
}

// 支持printf的重定向函数
int fputc(int ch, FILE* f) {
	 while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    USART_SendData(USART1, (uint8_t)ch);
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
    return ch;
}
