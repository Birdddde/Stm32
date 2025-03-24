#ifndef __STORAGE_H
#define __STORAGE_H

#include "stm32f10x.h"

// 存储相关常量定义
#define CARD_ID_SIZE             4
#define INVALID_SUM              0xFFFF
#define MAX_CARDS               16384    // 最大支持的卡片数量
#define SECTOR_SIZE             4096     // 扇区大小
#define BLOCK_SIZE              0x10000  // 块大小
#define FLASH_TOTAL_SIZE        0x800000 // Flash总大小
#define PAGE_SIZE               256      // 页编程大小
#define CHECKSUM_ADD				  0x400000 // 校验和存储地址
#define CARD_ADD					  0x410000 // 卡存储地址

// 错误码定义
typedef enum {
    FLASH_OK = 0,               // 操作成功
    FLASH_INVALID_PARAM,        // 无效参数
    FLASH_CARD_EXISTS,          // 卡片已存在
    FLASH_BUFFER_OVERFLOW,      // 缓冲区溢出
    FLASH_WRITE_ERROR,          // 写入错误
    FLASH_READ_ERROR,           // 读取错误
    FLASH_ERASE_ERROR,          // 擦除错误
    FLASH_TIMEOUT_ERROR         // 超时错误
} Flash_Status_t;


// 函数声明
void Storage_Init(void);
Flash_Status_t RC522_ReadCardID(uint16_t Cnt, uint8_t* CardID);
Flash_Status_t RC522_WriteIDToFlash(uint8_t* CardID);
Flash_Status_t RC522_Delete(uint16_t Cnt);
uint8_t RC522_ID_IsExist(uint8_t* CardID_Check, uint16_t* Cnt);
void RC522_EraseAll(void);
uint16_t RC522_GetSum(void);
void RC522_WriteSumToFlash(uint16_t Sum);

#endif
