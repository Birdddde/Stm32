#include "stm32f10x.h"                  // Device header
#include "freertos.h"
#include "timers.h"
#include "key.h"
#include "esp01s_wifimoudle.h"

xTimerHandle g_xKeyScanTimer,g_xUpdateTimer;  // 定时器句柄
extern uint8_t g_ucDoor_status;

// 定时器回调函数
void vTimerCallback(xTimerHandle pxTimer)
{
	uint32_t ulTimerID;
	configASSERT(pxTimer);
	
	ulTimerID = (uint32_t)pvTimerGetTimerID(pxTimer);
	
	if(ulTimerID ==0)
		vKeyScan();  
	
	if(ulTimerID ==1)
		MQTT_UploadState(g_ucDoor_status);  	
}

// 创建定时器
void Timer_Create(void)
{

    const TickType_t xTimerPeriod[2] = {pdMS_TO_TICKS(20),pdMS_TO_TICKS(10000)};  // 定时器周期40ms
	 
    g_xKeyScanTimer = xTimerCreate("KeyScanTimer",  // 定时器名称
                                    xTimerPeriod[0],    // 定时器周期
                                    pdTRUE,          // 周期性定时器
                                    (void*)0,             // 定时器 ID
                                    vTimerCallback); // 回调函数
	 if (g_xKeyScanTimer!= NULL) {
        xTimerStart(g_xKeyScanTimer, 0);  // 启动定时器
    }
	 
    g_xUpdateTimer = xTimerCreate("UpdateTimer",  // 定时器名称
                                    xTimerPeriod[1],    // 定时器周期
                                    pdTRUE,          // 周期性定时器
                                    (void*)1,             // 定时器 ID
                                    vTimerCallback); // 回调函数
	 if (g_xUpdateTimer!= NULL) {
        xTimerStart(g_xUpdateTimer, 0);  // 启动定时器
    }	 
}


