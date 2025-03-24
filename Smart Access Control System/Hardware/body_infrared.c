#include "stm32f10x.h"
#include "body_infrared.h"

void Body_Infra_Init(void)
{
		GPIO_InitTypeDef GPIO_InitStructure;
		
		RCC_APB2PeriphClockCmd (BODY_INFRA_GPIO_CLK, ENABLE );	// 打开连接 传感器DO 的单片机引脚端口时钟
		GPIO_InitStructure.GPIO_Pin = BODY_INFRA_GPIO_PIN;			// 配置连接 传感器DO 的单片机引脚模式
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;			// 设置为下拉输入
		
		GPIO_Init(BODY_INFRA_GPIO_PORT, &GPIO_InitStructure);				// 初始化 
	
}
 
uint16_t Body_Infra_GetData(void)
{
	uint16_t tempData;
	tempData = GPIO_ReadInputDataBit(BODY_INFRA_GPIO_PORT, BODY_INFRA_GPIO_PIN);
	return tempData;
}














