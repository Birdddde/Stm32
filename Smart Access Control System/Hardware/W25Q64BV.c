#include "stm32f10x.h"                  // Device header
#include "W25Q64BV_Ins.h"

#define PIN_SS GPIO_Pin_12
#define PIN_SCK GPIO_Pin_13
#define PIN_MISO GPIO_Pin_14
#define PIN_MOSI GPIO_Pin_15

void MySPI_W_SS(uint8_t BitValue)
{
	GPIO_WriteBit(GPIOB,PIN_SS,(BitAction)BitValue);
}

void MySPI_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2,ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = PIN_SCK | PIN_MOSI;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = PIN_MISO;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
		
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = PIN_SS;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	SPI_InitTypeDef SPI_InitStructure;
	SPI_InitStructure.SPI_BaudRatePrescaler=SPI_BaudRatePrescaler_128;
	SPI_InitStructure.SPI_CPHA=SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_CPOL=SPI_CPOL_Low;
	SPI_InitStructure.SPI_DataSize=SPI_DataSize_8b;
	SPI_InitStructure.SPI_Direction=SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_FirstBit=SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_Mode=SPI_Mode_Master;
	SPI_InitStructure.SPI_NSS=SPI_NSS_Soft;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI2,&SPI_InitStructure);
	
	
	SPI_Cmd(SPI2,ENABLE);
	MySPI_W_SS(1);
}

void MySPI_Start(void)
{
	MySPI_W_SS(0);
}
void MySPI_StoP(void)
{
	MySPI_W_SS(1);
}

uint8_t MySPI_SwapByte(uint8_t Command)
{
	uint8_t Data=0x00;

	while(!SPI_I2S_GetFlagStatus(SPI2,SPI_I2S_FLAG_TXE));//等待发送缓冲器空
	SPI_I2S_SendData(SPI2,Command);
	while(!SPI_I2S_GetFlagStatus(SPI2,SPI_I2S_FLAG_RXNE));//等待接收缓冲器非空
	Data=SPI_I2S_ReceiveData(SPI2);
	return Data;
}

void W25Q64BV_GetID(uint8_t *MID,uint16_t *DID)
{
    MySPI_Start();
    MySPI_SwapByte(W25Q64_JEDEC_ID);
    *MID=MySPI_SwapByte(W25Q64_DUMMY_BYTE);
    *DID=MySPI_SwapByte(W25Q64_DUMMY_BYTE);
    (*DID)<<=8;
    *DID|=MySPI_SwapByte(W25Q64_DUMMY_BYTE);
    MySPI_StoP();
}

void W25Q64BV_WriteEnable(void)
{
    MySPI_Start();
    MySPI_SwapByte(W25Q64_WRITE_ENABLE);
    MySPI_StoP();
}

void W25Q64BV_WriteDisable(void)
{
    MySPI_Start();
    MySPI_SwapByte(W25Q64_WRITE_DISABLE);
    MySPI_StoP();
}

void W25Q64BV_WaitBusy(void)
{
    uint32_t TimeOut=100000;
    MySPI_Start();
    MySPI_SwapByte(W25Q64_READ_STATUS_REGISTER_1);
    while((MySPI_SwapByte(W25Q64_DUMMY_BYTE)&0x01)==0x01)
    {
        TimeOut--;
        if(TimeOut<=0){break;}
    }
    MySPI_StoP();	
}

void W25Q64BV_SectorErase(uint32_t Address)
{
    W25Q64BV_WriteEnable();
    MySPI_Start();
    MySPI_SwapByte(W25Q64_SECTOR_ERASE_4KB);
    MySPI_SwapByte(Address>>16);
    MySPI_SwapByte(Address>>8);
    MySPI_SwapByte(Address);	
    MySPI_StoP();
    W25Q64BV_WaitBusy();
    W25Q64BV_WriteDisable();
}

void W25Q64BV_PageProgram(uint32_t Address,uint8_t* Data,uint16_t Length)
{
    uint16_t i;
    W25Q64BV_WriteEnable();
    
    MySPI_Start();
    MySPI_SwapByte(W25Q64_PAGE_PROGRAM);
    MySPI_SwapByte(Address>>16);
    MySPI_SwapByte(Address>>8);
    MySPI_SwapByte(Address);
    for(i=0;i<Length;i++)
    {
        MySPI_SwapByte(Data[i]);	
    }
    MySPI_StoP();
    
    W25Q64BV_WaitBusy();
    W25Q64BV_WriteDisable();
}

void W25Q64BV_ReadData(uint32_t Address,uint8_t* DataArray,uint16_t Count)
{
    uint16_t i;
    
    MySPI_Start();
    MySPI_SwapByte(W25Q64_READ_DATA);
    MySPI_SwapByte(Address>>16);
    MySPI_SwapByte(Address>>8);
    MySPI_SwapByte(Address);
    for(i=0;i<Count;i++)
    {
        DataArray[i]=MySPI_SwapByte(W25Q64_DUMMY_BYTE);
    }
    MySPI_StoP();
}

void W25Q64BV_Init(void)
{
    MySPI_Init();
	
}

