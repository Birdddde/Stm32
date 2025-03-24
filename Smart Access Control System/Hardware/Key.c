#include "stm32f10x.h"                  // Device header
#include "key.h"
#include "oled.h"
#include "freertos.h"
#include "semphr.h"  

#define GPIO_READ_PIN(x, y) GPIO_ReadInputDataBit(x, y)

extern SemaphoreHandle_t g_xMutex_Key;
//xTimerHandle g_xKeyScanTimer;  // 定时器句柄
uint8_t g_ucKeyNum = 0;

/* @note:可使用EXTI让响应更快 */
void Key_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);//使能AFIO钟
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);//使能GPIOB时钟
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
}

uint8_t KeyNum_Get_Callback(void)
{
	if	(GPIO_READ_PIN(GPIOB, GPIO_Pin_3) == 0)	return 4;
	if	(GPIO_READ_PIN(GPIOB, GPIO_Pin_4) == 0)	return 3;
	if	(GPIO_READ_PIN(GPIOB, GPIO_Pin_5) == 0)	return 2;
	if	(GPIO_READ_PIN(GPIOB, GPIO_Pin_6) == 0)	return 1;
	
	return 0;
}

// 按键扫描函数
void vKeyScan(void)
{
	static uint8_t lastState = 1;  // 记录上次按键状态
	static uint8_t currentState = 1;
	currentState = GPIO_READ_PIN(GPIOB,GPIO_Pin_3) & GPIO_READ_PIN(GPIOB,GPIO_Pin_4) 
	& GPIO_READ_PIN(GPIOB,GPIO_Pin_5) & GPIO_READ_PIN(GPIOB,GPIO_Pin_6);  // 读取按键状态

	if (currentState == 0 && lastState == 1) {
		// 按键被按下
		g_ucKeyNum = KeyNum_Get_Callback();
	} else if (currentState == 0 && lastState == 1) {
	  // 按键被释放
	}

	lastState = currentState;  // 更新上次按键状态
}

//// 定时器回调函数
//void vKeyTimerCallback(xTimerHandle pxTimer)
//{
//    vKeyScan();  // 每次定时器回调时扫描按键
//}

//// 创建定时器
//void KeyScanTimer_Create(void)
//{
//    const TickType_t xTimerPeriod = pdMS_TO_TICKS(20);  // 定时器周期40ms

//    g_xKeyScanTimer = xTimerCreate("KeyScanTimer",  // 定时器名称
//                                    xTimerPeriod,    // 定时器周期
//                                    pdTRUE,          // 周期性定时器
//                                    NULL,             // 定时器 ID
//                                    vKeyTimerCallback); // 回调函数
//	 if (g_xKeyScanTimer!= NULL) {
//        xTimerStart(g_xKeyScanTimer, 0);  // 启动定时器
//    }
//}

uint8_t KeyNum_Get(void)
{
	uint8_t KeyNum = 0;
//	xSemaphoreGive(g_xMutex_Key);
	if(g_ucKeyNum != 0){
		KeyNum = g_ucKeyNum;
		g_ucKeyNum = 0;
	}
//	xSemaphoreTake(g_xMutex_Key,0);
	return KeyNum;
}
