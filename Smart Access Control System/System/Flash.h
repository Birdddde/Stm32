#include "stm32f10x.h"                  // Device header
#ifndef __DELAY_H
#define __DELAY_H

#define STM32_SECTOR_SIZE	1024	//页大小
#define STM32_SECTOR_NUM	64		//页数

#define STM32_FLASH_BASE 0x08000000 //STM32 Flash起始地址
#define STM32_FLASH_END  0x08010000	//STM32 Flash结束地址

#define STM32_FLASH_FINGERCNT 0x0800FC00	//STM32 存储地址
#define STM32_FLASH_FACECNT  0x0800FC02	//STM32 存储地址

uint16_t FLASH_ReadHalfWord(uint32_t Addr);
void FLASH_WriteNByte(uint32_t Addr,uint8_t *pBuff,uint32_t Len);

#endif




