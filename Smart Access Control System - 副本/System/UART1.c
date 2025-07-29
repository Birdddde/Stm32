#include "string.h"
#include "uart1.h"
#include "freertos.h"
#include "task.h"

uint8_t rx_buffer[RX_BUFFER_SIZE];      // DMA直接操作的缓冲区
uint8_t rx_data_ready = 0;              // 数据接收完成标志
volatile uint16_t rx_data_length = 0;   // 实际接收数据长度

/**
 * @brief UART1初始化
 * @param baudrate 波特率
 */
void Serial1_Init(uint32_t baudrate) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

    // GPIO配置
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;       // PA9: USART1_TX
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;      // PA10: USART1_RX
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    // USART1配置
    USART_InitTypeDef USART_InitStruct;
    USART_InitStruct.USART_BaudRate = baudrate;
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;
    USART_InitStruct.USART_StopBits = USART_StopBits_1;
    USART_InitStruct.USART_Parity = USART_Parity_No;
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStruct);

    // 启用空闲中断
    USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);
    NVIC_EnableIRQ(USART1_IRQn);

    USART_Cmd(USART1, ENABLE);
}

/**
 * @brief DMA1初始化
 */
void DMA1_Init(void) {
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    DMA_InitTypeDef DMA_InitStruct;
    DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DR;
    DMA_InitStruct.DMA_MemoryBaseAddr = (uint32_t)rx_buffer;
    DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStruct.DMA_BufferSize = RX_BUFFER_SIZE;
    DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStruct.DMA_Mode = DMA_Mode_Normal;    // 普通模式
    DMA_InitStruct.DMA_Priority = DMA_Priority_High;
    DMA_InitStruct.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel5, &DMA_InitStruct);

    // 关联DMA与UART
    USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);
    DMA_Cmd(DMA1_Channel5, ENABLE);
}

/**
 * @brief 获取UART1接收数据
 * @param uart_Rx 接收数据结构体
 * @return 1: 接收成功, 0: 接收失败
 */
uint8_t USART1_GetRxData(Uart_Rx_t* uart_Rx){
	
	if(rx_data_ready){
		rx_data_ready = 0;
		uart_Rx -> rx_buffer = rx_buffer;
		uart_Rx -> rx_data_length = rx_data_length;
		return 1;
	}
	
	return 0;
}

/**
 * @brief 发送字符串
 * @param String 字符串
 */
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

/**
 * @brief USART1中断服务函数
 */
void USART1_IRQHandler(void) {
    if (USART_GetITStatus(USART1, USART_IT_IDLE) != RESET) {
        USART_ReceiveData(USART1);  // 清除空闲中断标志

        // 计算接收数据长度
        rx_data_length = RX_BUFFER_SIZE - DMA_GetCurrDataCounter(DMA1_Channel5);
        rx_data_ready = 1;          // 置接收完成标志

        // 重启DMA传输
        DMA_Cmd(DMA1_Channel5, DISABLE);
        DMA_SetCurrDataCounter(DMA1_Channel5, RX_BUFFER_SIZE);
        DMA_Cmd(DMA1_Channel5, ENABLE);
    }
}

