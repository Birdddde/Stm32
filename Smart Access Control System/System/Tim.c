#include "stm32f10x.h"                  // Device header
#include "freertos.h"
#include "timers.h"
#include "key.h"
#include "esp01s_wifimoudle.h"

xTimerHandle g_xKeyScanTimer,g_xUpdateTimer,g_xPassLockTimer,g_xPassRetryTimer,g_xWifiTimer;  // 定时器句柄
extern uint8_t g_ucDoor_status;
extern wifi_error_t g_xError;
extern uint8_t pass_retry;
extern uint8_t g_ucWifi_testcon_flag;

/**
  * @brief  定时器回调函数
  * @param  pxTimer 定时器句柄
  * @retval 无
  */
void vTimerCallback(xTimerHandle pxTimer)
{
	uint32_t ulTimerID;
	configASSERT(pxTimer);
	
	ulTimerID = (uint32_t)pvTimerGetTimerID(pxTimer);
	
	if(ulTimerID ==0)
		vKeyScan();  
	
//	if(ulTimerID ==1)
//		MQTT_UploadState(g_ucDoor_status);  	
	
	if(ulTimerID ==3)
		pass_retry = 0;
		 
	if(ulTimerID ==4)
		g_ucWifi_testcon_flag = 1;
}

/**
  * @brief  创建定时器
  * @param  无
  * @retval 无
  */
void Timer_Create(void)
{

    const TickType_t xTimerPeriod[5] = {pdMS_TO_TICKS(60),pdMS_TO_TICKS(10000),pdMS_TO_TICKS(60000),pdMS_TO_TICKS(600000),pdMS_TO_TICKS(3000000)};  // 定时器周期40ms
	 
    g_xKeyScanTimer = xTimerCreate("KeyScanTimer",  // 定时器名称
                                    xTimerPeriod[0],    // 定时器周期
                                    pdTRUE,          // 周期性定时器
                                    (void*)0,             // 定时器 ID
                                    vTimerCallback); // 回调函数
	 if (g_xKeyScanTimer!= NULL) {
        xTimerStart(g_xKeyScanTimer, 0);  // 启动定时器
    }
	 
	 g_xWifiTimer = xTimerCreate("WifiTimer",  // 定时器名称
                                    xTimerPeriod[4],    // 定时器周期
                                    pdTRUE,          // 周期性定时器
                                    (void*)4,             // 定时器 ID
                                    vTimerCallback); // 回调函数
	 if (g_xWifiTimer!= NULL) {
        xTimerStart(g_xWifiTimer, 0);  // 启动定时器
    }
	 
#ifdef WIFI
	 if(!g_xError.error_code){
		 g_xUpdateTimer = xTimerCreate("UpdateTimer",  // 定时器名称
											xTimerPeriod[1],    // 定时器周期
											pdTRUE,          // 周期性定时器
											(void*)1,             // 定时器 ID
											vTimerCallback); // 回调函数
		 if (g_xUpdateTimer!= NULL) {
			  xTimerStart(g_xUpdateTimer, 0);  // 启动定时器
		 }
	 } 
#endif
	 
	 g_xPassLockTimer = xTimerCreate("PassTimer",  // 定时器名称
                                    xTimerPeriod[2],    // 定时器周期
                                    pdFALSE,          // 周期性定时器
                                    (void*)2,             // 定时器 ID
                                    vTimerCallback); // 回调函数
	
	 g_xPassRetryTimer = xTimerCreate("PassRetry",  // 定时器名称
                                    xTimerPeriod[3],    // 定时器周期
                                    pdFALSE,          // 周期性定时器
                                    (void*)3,             // 定时器 ID
                                    vTimerCallback); // 回调函数
	 
}


