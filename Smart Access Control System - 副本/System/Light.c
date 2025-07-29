#include "stm32f10x.h"

/**
  * @brief  LED灯初始化
  * @param  无
  * @retval 无
  * @note   使用PA12引脚作为LED控制引脚
  *         高电平点亮LED，低电平熄灭LED
  */
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

/**
  * @brief  LED灯点亮
  * @param  无
  * @retval 无
  * @note   使用PA12引脚作为LED控制引脚
  *         高电平点亮LED，低电平熄灭LED
  */
void Light_On(void){
	GPIO_SetBits(GPIOA,GPIO_Pin_12);
}

/**
  * @brief  LED灯熄灭
  * @param  无
  * @retval 无
  * @note   使用PA12引脚作为LED控制引脚
  *         高电平点亮LED，低电平熄灭LED
  */
void Light_Off(void){
	GPIO_ResetBits(GPIOA,GPIO_Pin_12);
}
