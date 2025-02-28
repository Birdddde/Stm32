#include "stm32f10x.h"
#include <stdio.h>
#include "string.h"
#include "flash.h"
#include "uart1.h"

/*F103C8T6 
Flash:64KB  0x0800 0000 -> 0x8000 FFFF
SRAM :20KB  0x2000 0000 -> 0x2005 0000

分区		start_add		size		page_start
BOOT		0x0800 0000		15kb		0
VER		0x0800 3C00		1kb		15
APP		0x0800 4000		24kb		16
OTA		0x0800 A000		24kb		40
*/

#define BOOTLOADER_SIZE    0x3C00      // 15KB
#define APP_VERSION_SIZE   0x0400      // 1KB
#define APP_SIZE				0X6000		// 24KB
#define IAP_SIZE				0X6000		// 24KB

// 修正分区地址
#define BOOTLOADER_ADDRESS  FLASH_BASE          // 0x08000000
#define APP_VERSION_ADDRESS (FLASH_BASE + BOOTLOADER_SIZE) // 0x08003C00
#define APP_ADDRESS         (APP_VERSION_ADDRESS + APP_VERSION_SIZE) // 0x08004000
#define OTA_ADDRESS         (APP_ADDRESS + APP_SIZE)        // 0x0800A000

// 修改后的JumpToApp
void JumpToApp(void) {
    typedef void (*pFunction)(void);
    pFunction Jump_To_Application;
    
    __disable_irq(); // 先关闭中断
    
    if ((*(volatile uint32_t*)APP_ADDRESS & 0x2FFE0000) == 0x20000000) {
        __set_MSP(*(volatile uint32_t*)APP_ADDRESS);
        Jump_To_Application = (pFunction)*(volatile uint32_t*)(APP_ADDRESS + 4);
        Jump_To_Application();
    } else {
        printf("APP check failed!\n");
        NVIC_SystemReset();
    }
}

void erase_ota_area(void) {
    Flash_EraseSector(40,64-1);
}

void erase_app_area(void) {
    Flash_EraseSector(16,40-1);
}

void copy_ota_to_app(void) {
    // 擦除APP区域
    erase_app_area();
    
    // 从OTA区域复制到APP区域
	 uint32_t src = FLASH_ReadWord((uint32_t)OTA_ADDRESS);
	 uint32_t dest = FLASH_ReadWord((uint32_t)APP_ADDRESS);
	 
    uint32_t words_to_copy = IAP_SIZE / 4; // 按字复制
    
    FLASH_Unlock();
    for(int i=0; i<words_to_copy; i++) {
        FLASH_ProgramWord(dest + i, FLASH_ReadWord(src + i));
    }
    FLASH_Lock();
}

void perform_ota_update(void) {
    uint32_t firmware_size = 0;
    uint32_t expected_crc = 0;
    
    // 1. 擦除OTA区域
    erase_ota_area();
    
    // 2. 接收固件
    if (receive_firmware_header(&firmware_size, &expected_crc) == SUCCESS) {
        receive_firmware_data(firmware_size);
    } else {
        uart1_send_string("Header Error\n");
        return;
    }
    
    // 3. 校验固件
    if (validate_firmware(firmware_size, expected_crc)) {
        // 4. 复制到APP区域
        copy_ota_to_app(firmware_size);
        uart1_send_string("OTA Success!\n");
        
        // 5. 更新版本信息（可选）
        update_version_info();
        
        // 6. 重启系统
        NVIC_SystemReset();
    } else {
        uart1_send_string("CRC Error\n");
        // 保留旧版本，下次启动仍可跳转APP
    }
}

// 完善main函数
int main(void) {
    SystemInit();
    Serial1_Init(115200);
    uint8_t Flag_Flash = 0; 
	
    // 示例：检查GPIO或Flash标志决定是否升级
    if (need_ota_update()) {
        perform_ota_update();
    } else {
        JumpToApp();
    }

    while(1); // 正常情况下不应到达此处
}

