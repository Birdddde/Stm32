#include "flash.h"

/**
 * @brief  读取一个半字
 * @param  Addr: 读取地址
 * @retval: 读取到的数据
 */
uint16_t FLASH_ReadHalfWord(uint32_t Addr)
{
	return *((__IO uint16_t *)(Addr));
}

/**
 * @brief  写入N个字节
 * @param  Addr: 写入地址
 * @param  pBuff: 数据缓冲区
 * @param  Len: 写入长度
 * @retval: 无
 */
void FLASH_WriteNByte(uint32_t Addr,uint8_t *pBuff,uint32_t Len)
{
	uint16_t i;
	uint16_t temp = 0;
	
	if((Addr < STM32_FLASH_BASE) || (Addr > STM32_FLASH_END))
		return;
	
	FLASH_Unlock();//解锁
	
	for(i = 0;i < Len;i += 2)
	{
		temp = pBuff[i];
		temp |= (uint16_t)pBuff[i+1] << 8;
		
		FLASH_ProgramHalfWord(Addr,temp);
		Addr += 2;
		if(Addr > STM32_FLASH_END)
		{
			FLASH_Lock();
			return;
		}
	}
	FLASH_Lock();
}
