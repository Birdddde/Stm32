#include "stm32f10x.h"

void Light_Init(void)
{

   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); // 确保已执行

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	GPIO_ResetBits(GPIOA,GPIO_Pin_12);
}

void Light_On(void){
	GPIO_SetBits(GPIOA,GPIO_Pin_12);
}

void Light_Off(void){
	GPIO_ResetBits(GPIOA,GPIO_Pin_12);
}
