#include "stm32f10x.h"                  // Device header
#ifndef __DELAY_H
#define __DELAY_H

#define STM32_SECTOR_SIZE	1024	//页大小
#define STM32_SECTOR_NUM	127		//页数

#define STM32_FLASH_BASE 0x08000000 //STM32 Flash起始地址
#define STM32_FLASH_END  0x0801FFFF	//STM32 Flash结束地址

uint8_t FLASH_ReadByte(uint32_t Addr);
uint16_t FLASH_ReadHalfWord(uint32_t Addr);
uint32_t FLASH_ReadWord(uint32_t Addr);
void FLASH_ReadNByte(uint32_t Addr,uint8_t *pBuff,uint32_t Len);
void FLASH_ReadPage(uint8_t Page_Num,uint8_t *pBuff);
void FLASH_WritePage(uint8_t Page_Num,uint8_t *pBuff);
void FLASH_WriteNData(uint32_t Addr,uint8_t *pBuff,uint32_t Len);
void FLASH_WriteNByte(uint32_t Addr,uint8_t *pBuff,uint32_t Len);
void Flash_EraseSector(uint8_t Start_Page,uint8_t End_Page);

#endif




