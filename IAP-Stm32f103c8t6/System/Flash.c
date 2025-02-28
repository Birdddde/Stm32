#include "flash.h"

uint8_t STM32_FLASH_BUFF[STM32_SECTOR_SIZE]={0};

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
 * @brief  读取一个字
 * @param  Addr: 读取地址
 * @retval: 读取到的数据
 */
uint32_t FLASH_ReadWord(uint32_t Addr){
	return *((__IO uint32_t *)(Addr));
}

/**
 * @brief  读N个字节
 * @param  Addr: 写入地址
 * @param  pBuff: 数据缓冲区
 * @param  Len: 读取长度
 * @retval: 无
 */
void FLASH_ReadNByte(uint32_t Addr,uint8_t *pBuff,uint32_t Len)
{
	uint32_t i;
	
	for(i = 0;i < Len;i++)
	{
		pBuff[i] = FLASH_ReadByte(Addr);
		Addr += 1;
	}
}

/**
 * @brief  读一页数据
 * @param  Page_Num: 页号
 * @param  pBuff: 数据缓冲区
 * @retval: 无
 */
void FLASH_ReadPage(uint8_t Page_Num,uint8_t *pBuff)
{
	uint16_t i;
	uint32_t Buff;
	uint32_t Addr;
	
	//是否超出范围
	if(Page_Num > STM32_SECTOR_NUM)
		return;
	
	//先计算页首地址
	Addr = Page_Num * STM32_SECTOR_SIZE + STM32_FLASH_BASE;
	
	for(i = 0;i < STM32_SECTOR_SIZE;i += 4)
	{
		Buff = FLASH_ReadWord(Addr);
		
		pBuff[i]   = Buff;
		pBuff[i+1] = Buff >> 8;
		pBuff[i+2] = Buff >> 16;
		pBuff[i+3] = Buff >> 24;
		
		Addr += 4;
	}
}

/**
 * @brief  写入页
 * @param  Page_Num: 页号
 * @param  pBuff: 数据缓冲区
 * @retval: 无
 */
void FLASH_WritePage(uint8_t Page_Num,uint8_t *pBuff)
{
	uint16_t i;
	uint16_t Buff;
	uint32_t Addr;
	

	if(Page_Num > STM32_SECTOR_NUM)
		return;

	FLASH_Unlock();

	Addr = Page_Num * STM32_SECTOR_SIZE + STM32_FLASH_BASE;
	
	for(i = 0;i < STM32_SECTOR_SIZE ;i += 2)
	{
		Buff = ((uint16_t)pBuff[i+1] << 8) | pBuff[i];
		FLASH_ProgramHalfWord(Addr,Buff);
		Addr += 2;
	}

	FLASH_Lock();
}

/**
 * @brief  写入N个字节
 * @param  Addr: 写入地址
 * @param  pBuff: 数据缓冲区
 * @param  Len: 写入长度
 * @retval: 无
 */
void FLASH_WriteNData(uint32_t Addr,uint8_t *pBuff,uint32_t Len)
{
	uint32_t Offset;
	uint8_t  Page_Num;
	uint16_t Page_Offset;
	uint16_t Free_Space;
	uint16_t i;
	
	if((Addr < STM32_FLASH_BASE) || (Addr > STM32_FLASH_END))
		return;
	
	Offset = Addr - STM32_FLASH_BASE;//偏移地址
	Page_Num = Offset / STM32_SECTOR_SIZE;//得到地址所在页
	Page_Offset = Offset % STM32_SECTOR_SIZE;//在页内的偏移地址
	Free_Space = STM32_SECTOR_SIZE -  Page_Offset;//页区剩余空间
	//要写入的数据是否大于剩余空间
	if(Len <= Free_Space)
		Free_Space = Len;
	
	FLASH_Unlock();//解锁
	
	while(1)
	{
		FLASH_ReadPage(Page_Num,STM32_FLASH_BUFF);//先把数据读到缓存中
		FLASH_ErasePage(Page_Num * STM32_SECTOR_SIZE + STM32_FLASH_BASE);//页擦除
		//修改缓存数据
		for(i = 0;i < Free_Space;i++)
		{
			STM32_FLASH_BUFF[i+Page_Offset] = pBuff[i];
		}
		FLASH_WritePage(Page_Num,STM32_FLASH_BUFF);//把缓存数据写入
		//判断是否超出当前页，超出进入下一页
		if(Len == Free_Space)
			break;
		else
		{
			Page_Num++;//下一页
			Page_Offset = 0;
			pBuff += Free_Space;
			
			Len -= Free_Space;
			if(Len > STM32_SECTOR_SIZE)
				Free_Space = STM32_SECTOR_SIZE;
			else
				Free_Space = Len;
		}
	}
	FLASH_Lock();
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


void Flash_EraseSector(uint8_t Start_Page,uint8_t End_Page)
{
	uint8_t i;
	uint8_t num = 0;
	
	if(Start_Page > End_Page)
		return;
	
	FLASH_Unlock();//解锁
	
	num = End_Page - Start_Page;//擦除页数
	
	for(i = 0;i <= num;i++)
	{
		FLASH_ErasePage((Start_Page + i) * STM32_SECTOR_SIZE + STM32_FLASH_BASE);//页擦除
	}
	
	FLASH_Lock();
}


