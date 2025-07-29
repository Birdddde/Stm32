#include "stm32f10x.h"                  // Device header
#include "w25q64bv.h"                  // Device header
#include "W25Q64BV_Ins.h"
#include "storage.h"
#include "string.h"

#define DEBUG
#ifdef DEBUG
    #include "stdio.h"
#endif

#define CARD_ID_SIZE 4
#define INVALID_SUM 0xFFFF

static uint8_t sector_buffer[4096]={0xff};			
static uint16_t g_ulSum = 0;
extern uint16_t g_usFingerId;
extern uint8_t g_ucaAdmin_pass[4];
uint8_t Admin_Default[4] = {0,0,0,0};

// 内部函数声明
static Flash_Status_t CheckAddress(uint32_t address);
static Flash_Status_t WriteToSector(uint32_t sector_address, uint8_t* data, uint32_t length);

/**
 * @brief 获取已存储的卡片数量
 * @return uint16_t 卡片数量，0xFFFF表示无效
 */
uint16_t RC522_GetSum(void)
{
    uint32_t Address = CHECKSUM_ADD;
    uint16_t Sum = 0;
    
    if (CheckAddress(Address) != FLASH_OK) {
        return INVALID_SUM;
    }
    
    MySPI_Start();
    MySPI_SwapByte(W25Q64_READ_DATA);
    MySPI_SwapByte(Address >> 16);
    MySPI_SwapByte(Address >> 8);
    MySPI_SwapByte(Address);
    
    Sum = MySPI_SwapByte(W25Q64_DUMMY_BYTE);
    Sum |= (uint16_t)MySPI_SwapByte(W25Q64_DUMMY_BYTE) << 8;
    
    MySPI_StoP();
    return Sum;   
}

/**
 * @brief 将卡片数量写入Flash
 * @param Sum 卡片数量（1-16384）
 */
void RC522_WriteSumToFlash(uint16_t Sum)
{
    uint32_t Address = CHECKSUM_ADD;
    uint8_t data[2];
    
    if (Sum > MAX_CARDS) {
        return;
    }
    
    data[0] = Sum & 0xFF;
    data[1] = (Sum >> 8) & 0xFF;
    
    W25Q64BV_SectorErase(Address);
    W25Q64BV_PageProgram(Address, data, 2);
}

/**
 * @brief 读取指定位置的卡号
 * @param Cnt 卡号位置（1-16384）
 * @param CardID 卡号存储缓冲区
 * @return Flash_Status_t 操作状态
 */
Flash_Status_t RC522_ReadCardID(uint16_t Cnt, uint8_t* CardID)
{
    if (Cnt == 0 || Cnt > MAX_CARDS || CardID == NULL)
        return FLASH_INVALID_PARAM;
    
    uint32_t address = CARD_ADD + (Cnt - 1) * CARD_ID_SIZE;
    if (CheckAddress(address) != FLASH_OK)
        return FLASH_INVALID_PARAM;
        
    W25Q64BV_ReadData(address, CardID, CARD_ID_SIZE);
    return FLASH_OK;
}

/**
 * @brief 写入新卡号到Flash
 * @param CardID 要写入的卡号
 * @return Flash_Status_t 操作状态
 */
Flash_Status_t RC522_WriteIDToFlash(uint8_t* CardID)
{
    uint16_t cnt;
    if (CardID == NULL)
        return FLASH_INVALID_PARAM;
        
    // 检查卡号是否已存在
    if (RC522_ID_IsExist(CardID, &cnt))
        return FLASH_CARD_EXISTS;
        
    if (g_ulSum >= MAX_CARDS)
        return FLASH_BUFFER_OVERFLOW;
        
    uint32_t address = CARD_ADD + g_ulSum * CARD_ID_SIZE;
    uint32_t sector_address = (address / SECTOR_SIZE) * SECTOR_SIZE;
    uint32_t offset = address - sector_address;
    
    // 读取整个扇区
    W25Q64BV_ReadData(sector_address, sector_buffer, SECTOR_SIZE);
    
    // 写入新卡号
    memcpy(&sector_buffer[offset], CardID, CARD_ID_SIZE);
    
    // 更新扇区
    Flash_Status_t status = WriteToSector(sector_address, sector_buffer, SECTOR_SIZE);
    if (status != FLASH_OK)
        return status;
        
    // 更新计数
    g_ulSum++;
    RC522_WriteSumToFlash(g_ulSum);
    
    return FLASH_OK;
}

/**
 * @brief 删除指定位置的卡号
 * @param Cnt 要删除的卡号位置（1-16384）
 * @return Flash_Status_t 操作状态
 */
Flash_Status_t RC522_Delete(uint16_t Cnt) 
{
    if (Cnt == 0 || Cnt > g_ulSum || g_ulSum == 0)
        return FLASH_INVALID_PARAM;

    uint32_t address = CARD_ADD + (Cnt - 1) * CARD_ID_SIZE;
    uint32_t sector_address = (address / SECTOR_SIZE) * SECTOR_SIZE;
    uint32_t offset = address - sector_address;

    // 读取整个扇区数据到缓冲区
    W25Q64BV_ReadData(sector_address, sector_buffer, SECTOR_SIZE);

    // 计算需要移动的数据长度
    uint32_t bytes_to_move = (g_ulSum - Cnt) * CARD_ID_SIZE;
    
    if (bytes_to_move > 0) {
        // 移动后续数据前移
        memmove(&sector_buffer[offset], 
                &sector_buffer[offset + CARD_ID_SIZE], 
                bytes_to_move);
    }

    // 将最后一个卡号位置清空
    uint32_t last_card_offset = CARD_ADD + (g_ulSum - 1) * CARD_ID_SIZE - sector_address;
    memset(&sector_buffer[last_card_offset], 0xFF, CARD_ID_SIZE);

    // 擦除扇区
    W25Q64BV_SectorErase(sector_address);
    W25Q64BV_WaitBusy();

    // 按页写入更新后的数据
    for (uint16_t i = 0; i < SECTOR_SIZE; i += PAGE_SIZE) {
        uint16_t write_size = ((SECTOR_SIZE - i) < PAGE_SIZE) ? (SECTOR_SIZE - i) : PAGE_SIZE;
        W25Q64BV_PageProgram(sector_address + i, &sector_buffer[i], write_size);
        W25Q64BV_WaitBusy();
    }

    // 更新计数
    g_ulSum--;
    RC522_WriteSumToFlash(g_ulSum);

    return FLASH_OK;
}

/**
 * @brief 检查卡号是否已存在
 * @param CardID_Check 要检查的卡号
 * @param Cnt 如果存在，返回卡号位置
 * @return uint8_t 1:存在 0:不存在
 */
uint8_t RC522_ID_IsExist(uint8_t* CardID_Check, uint16_t* Cnt) 
{
    if (CardID_Check == NULL || Cnt == NULL)
        return 0;

    uint8_t cardid_temp[CARD_ID_SIZE];
    
    for (uint16_t i = 1; i <= g_ulSum; i++) {
        if (RC522_ReadCardID(i, cardid_temp) != FLASH_OK)
            continue;
            
        if (memcmp(cardid_temp, CardID_Check, CARD_ID_SIZE) == 0) {
            *Cnt = i;
            return 1;
        }
    }
    return 0;
}

/**
 * @brief 擦除所有卡号数据
 */
void RC522_EraseAll(void)
{
    uint32_t Address = CARD_ADD;
    
    // 擦除64KB块
    W25Q64BV_WriteEnable();
    MySPI_Start();
    MySPI_SwapByte(W25Q64_BLOCK_ERASE_64KB);
    MySPI_SwapByte(Address >> 16);
    MySPI_SwapByte(Address >> 8);
    MySPI_SwapByte(Address);	
    MySPI_StoP();
    W25Q64BV_WaitBusy();
    W25Q64BV_WriteDisable();	
    
    // 重置计数器
    RC522_WriteSumToFlash(0);
    g_ulSum = 0;
}


/**
 * @brief 检查地址是否有效
 * @param address 要检查的地址
 * @return Flash_Status_t 操作状态
 */
static Flash_Status_t CheckAddress(uint32_t address)
{
    if (address >= FLASH_TOTAL_SIZE)
        return FLASH_INVALID_PARAM;
    return FLASH_OK;
}

/**
 * @brief 写入数据到扇区
 * @param sector_address 扇区地址
 * @param data 要写入的数据
 * @param length 数据长度
 * @return Flash_Status_t 操作状态
 */
static Flash_Status_t WriteToSector(uint32_t sector_address, uint8_t* data, uint32_t length)
{
    if (data == NULL || length > SECTOR_SIZE)
        return FLASH_INVALID_PARAM;
        
    // 擦除扇区
    W25Q64BV_SectorErase(sector_address);
    
    // 按页写入数据
    for (uint16_t i = 0; i < length; i += PAGE_SIZE) {
        uint16_t write_size = ((length - i) < PAGE_SIZE) ? (length - i) : PAGE_SIZE;
        W25Q64BV_PageProgram(sector_address + i, &data[i], write_size);
    }
    
    return FLASH_OK;
}

//******************************指纹与管理员****************************************//////
void Finger_WriteSumToFlash(uint16_t Sum)
{
    uint32_t Address = FINGERID;
    uint8_t data[2];
    
    if (Sum > MAX_FINGERS) {
        return;
    }
    
    data[0] = Sum & 0xFF;
    data[1] = (Sum >> 8) & 0xFF;
    
    W25Q64BV_SectorErase(Address);
    W25Q64BV_PageProgram(Address, data, 2);
}

uint16_t Finger_GetSum(void)
{
    uint32_t Address = FINGERID;
    uint16_t Sum = 0;
    
    if (CheckAddress(Address) != FLASH_OK) {
        return INVALID_SUM;
    }
    
    MySPI_Start();
    MySPI_SwapByte(W25Q64_READ_DATA);
    MySPI_SwapByte(Address >> 16);
    MySPI_SwapByte(Address >> 8);
    MySPI_SwapByte(Address);
    
    Sum = MySPI_SwapByte(W25Q64_DUMMY_BYTE);
    Sum |= (uint16_t)MySPI_SwapByte(W25Q64_DUMMY_BYTE) << 8;
    
    MySPI_StoP();
    return Sum;   
}

void Admin_WritePassToFlash(uint8_t* Pass)
{
    uint32_t Address = ADMIN_PASS;
    
    W25Q64BV_SectorErase(Address);
    W25Q64BV_PageProgram(Address, Pass, 4);
}

uint16_t Admin_GetPass(uint8_t *Pass)
{
    uint32_t Address = ADMIN_PASS;
    
    if (CheckAddress(Address) != FLASH_OK) {
        return INVALID_SUM;
    }
    
    MySPI_Start();
    MySPI_SwapByte(W25Q64_READ_DATA);
    MySPI_SwapByte(Address >> 16);
    MySPI_SwapByte(Address >> 8);
    MySPI_SwapByte(Address);
    
	 for (uint8_t i = 0; i < 4; i ++) {
        Pass[i] = MySPI_SwapByte(W25Q64_DUMMY_BYTE);
    }
    
    MySPI_StoP();
    return FLASH_OK;   
}


/**
 * @brief 存储系统初始化
 */
void Storage_Init(void)
{
	uint8_t Pass[4];
	
    // 初始化Flash
    W25Q64BV_Init();
    
    // 初始化计数器
    uint16_t sum = RC522_GetSum();
    if (sum == INVALID_SUM) {
        g_ulSum = 0;
        RC522_WriteSumToFlash(0);
    } else {
        g_ulSum = sum;
    }
	 
	 sum = Finger_GetSum();
    if (sum >= MAX_FINGERS) {
        g_usFingerId = 0;
        Finger_WriteSumToFlash(0);
    } else {
        g_usFingerId = sum;
    }
	 
	 Admin_GetPass(Pass);
	 if( Pass[0] == 0xFF|| Pass[1] == 0xFF || Pass[2] == 0xFF || Pass[3] == 0xFF)
	 {
		Admin_WritePassToFlash(Admin_Default);
	 }else
	 {
		Admin_GetPass(g_ucaAdmin_pass);
	 }
	 
}

