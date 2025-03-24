#ifndef __W25Q64BV_H
#define __W25Q64BV_H
	void W25Q64BV_Init(void);
	void W25Q64BV_GetID(uint8_t *MID,uint16_t *DID);
	void W25Q64BV_WriteEnable(void);
	void W25Q64BV_WriteDisable(void);
	void W25Q64BV_SectorErase(uint32_t Address);
	void W25Q64BV_PageProgram(uint32_t Address,uint8_t* Data,uint16_t Length);
	void W25Q64BV_ReadData(uint32_t Address,uint8_t* DataArray,uint16_t Count);
	uint16_t RC522_GetSum(void);
	void RC522_WriteSumToFlash(uint16_t Sum);
	void W25Q64BV_DMA_Init(void);

	void MySPI_Start(void);
	void MySPI_StoP(void);
	void W25Q64BV_WaitBusy(void);
	uint8_t MySPI_SwapByte(uint8_t Command);

#endif
