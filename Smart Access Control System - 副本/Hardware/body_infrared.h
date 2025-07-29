#include "stm32f10x.h"
#ifndef __BODY_INFRA_H
#define __BODY_INFRA_H

#define	BODY_INFRA_GPIO_CLK								RCC_APB2Periph_GPIOA
#define 	BODY_INFRA_GPIO_PORT								GPIOA
#define 	BODY_INFRA_GPIO_PIN								GPIO_Pin_11		

void Body_Infra_Init(void);
uint16_t Body_Infra_GetData(void);
 
#endif
