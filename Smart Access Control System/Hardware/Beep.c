#include "stm32f10x.h"                  // Device header
#include "freertos.h"
#include "task.h"

void Beep_Init(void){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);	
	GPIO_SetBits(GPIOC, GPIO_Pin_13);
}

void Beep_On(uint16_t ms){

	GPIO_ResetBits(GPIOC, GPIO_Pin_13);
	vTaskDelay(pdMS_TO_TICKS(ms));
	GPIO_SetBits(GPIOC, GPIO_Pin_13);
	vTaskDelay(pdMS_TO_TICKS(ms));
	
}
