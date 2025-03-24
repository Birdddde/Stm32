#include "stm32f10x.h"
#include <stdio.h>

#include "freertos.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"  
#include "tim.h"

#include "oled.h"
#include "face.h"
#include "finger.h"
#include "rfid.h"
#include "servo.h"
#include "beep.h"
#include "light.h"
#include "esp01s_wifimoudle.h"
#include "body_infrared.h"
#include "key.h"
#include "typedef.h"
#include "menu.h"


#define MENU_MAIN_LENGTH 3

TaskHandle_t g_xRC522Handle = NULL;
TaskHandle_t g_xMenuHandle = NULL;
TaskHandle_t g_xAs608Handle = NULL;
TaskHandle_t g_xLightHandle = NULL;
TaskHandle_t g_xK210Handle = NULL;

wifi_error_t g_xError;				//阿里云连接错误标志位
uint8_t g_ucDoor_status = 0;		//门锁状态标志位

SemaphoreHandle_t g_xMutex_Wifi,g_xMutex_Key,g_xOledMutex;

volatile MenuState_t menuState = MENU_INACTIVE;
extern QueueHandle_t xQueueUart2;
uint8_t g_ucaAdmin_pass[4];
extern uint16_t g_usFingerId;

uint16_t g_usFaceId;
uint8_t g_uckey;	//按键

void AS608Task(void *p){
	as608_status_t as608_status;
	uint8_t status = 0;

	while(1)
	{
		if(xTaskNotifyWait(0x01, 0x01, NULL, pdMS_TO_TICKS(100)) == pdPASS) 
		{			
			vTaskSuspend(g_xMenuHandle);	
			OLED_Clear();
			OLED_ShowString(0,30,"Flushing Finger...",OLED_6X8);
			OLED_Update();
			
			status = Finger_Flush(&as608_status,NULL,NULL);		
			OLED_ClearArea(0,30,128,8);
			if( status == FLUSH_FINGER_SUCCESS ){
				OLED_ShowString(0,30,"found",OLED_6X8);
				OLED_UpdateArea(0,30,128,8);
				
			}				
			if( status == FLUSH_FINGER_SEARCH_FAILED ){				
				OLED_ShowString(0,30,"not found",OLED_6X8);
				OLED_UpdateArea(0,30,128,8);
				Beep_On(1000);		
			}
			
			OLED_UpdateArea(0,30,128,8);
			OLED_Clear();
			OLED_Update();
			if( status == FLUSH_FINGER_SUCCESS )
				xTaskNotify(g_xMenuHandle,0x02,eSetBits);
				
				
			vTaskResume(g_xMenuHandle);				
		}		
	}			
}

void MenuTask(void *p){

	uint32_t ulNotificationValue;
	uint8_t Flag_Access = 0;
	
	while(1){
		
		if(xTaskNotifyWait(0x03, 0x03, &ulNotificationValue, 0) == pdPASS) {
			if(ulNotificationValue & 0x01){
				vTaskSuspend(g_xRC522Handle);	//暂停任务
				vTaskSuspend(g_xAs608Handle);
				vTaskSuspend(g_xK210Handle);

				Menu_Init();
				menuState = MENU_ACTIVE;
				Display_Refresh();
			}
			if(ulNotificationValue & 0x02){
				Flag_Access = 1;
			}
		}
		
		
		if(menuState == MENU_ACTIVE){
			g_uckey = KeyNum_Get();
			if(g_uckey != KEY_NONE)	Key_Handler(g_uckey);

		   vTaskDelay(pdMS_TO_TICKS(10));
		}else
		{
			if(Flag_Access){
				Servo_Control_Angle(180);
				g_ucDoor_status = 1;
				vTaskDelay(pdMS_TO_TICKS(2000));
				Flag_Access=0;
			}else{
				OLED_ShowImage(48,16,32,32,Lock32x32);
				Servo_Control_Angle(0);
				g_ucDoor_status = 0;
			}			
			OLED_UpdateArea(48,16,32,32);
			
			g_uckey = KeyNum_Get();
			if(g_uckey){	
				Pass_handlle(g_ucaAdmin_pass,Action_COMAPRE);
			}
		}
		
		vTaskDelay(pdMS_TO_TICKS(100)); // 非激活状态时降低检测频
	}
}

void RC522Task(void *p){
	static uint8_t ucStatus;
	static uint8_t CardId[4]={0};
	
	while(1){

		ucStatus = RFID_Scan(CardId);	
		
		if(ucStatus){
			ucStatus = 0;
//			if(! RC522_ID_IsExist(CardId,&ucCard_cnt) )	
//			MQTT_UploadState(g_ucDoor_status);
			xTaskNotify(g_xMenuHandle,0x02,eSetBits);
		}
		vTaskDelay(pdMS_TO_TICKS(500)); // 非激活状态时降低检测频
	}
}

void K210Task(void *p){
	
	static uint8_t ucStatus;
	
	while(1){	
		ucStatus = Face_Scan();
		if(ucStatus){
			ucStatus = 0;
			xTaskNotify(g_xMenuHandle,0x02,eSetBits);
		}
			
		vTaskDelay(pdMS_TO_TICKS(2000)); // 非激活状态时降低检测频
	}
}

void LightTask(void *p){
	TickType_t Tick=pdMS_TO_TICKS(5000);
	
	while(1){

		if( Body_Infra_GetData() == 1 )
			Light_On();
		else
			Light_Off();
		
		vTaskDelay(Tick);
	}
	
}

int main(void)
{

	OLED_Init();
	RFID_Init();
   Key_Init();  
	Finger_Init();
	Face_Init();
	Beep_Init();
	Servo_Init();
	Light_Init();
	Body_Infra_Init();
//	ESP01S_Init();
	
	
//	g_xMutex_Wifi= xSemaphoreCreateMutex();	//创建互斥信号量	
	g_xMutex_Key = xSemaphoreCreateMutex();
	g_xOledMutex = xSemaphoreCreateMutex();
//	OLED_ShowString(30-6,32-8,"Connecting",OLED_8X16);
//	OLED_UpdateArea(30-6,32-8,128,16);
//	
//	Esp01s_ConnectAli(&g_xError);
//	OLED_Clear();	//清屏
//	OLED_Update();
	
//	if (g_xError.error_code)
//	{
		
//		OLED_ShowString(0,0,"Connect Aliyun failed",OLED_6X8);
//		OLED_ShowString(0,8,"error code:",OLED_6X8);
//		OLED_ShowString(0,16,"error src:",OLED_6X8);
//		OLED_ShowNum(13*6,8,g_xError.error_code,2,OLED_6X8);
//		OLED_ShowNum(13*6,16,g_xError.error_src,2,OLED_6X8);
//		OLED_UpdateArea(0,0,128,24);
//	}else
//	{
//		MQTT_UploadPass(g_ucaAdmin_pass);
		Timer_Create();
		xTaskCreate(MenuTask, "Menu", 128, NULL, 60, &g_xMenuHandle);
		xTaskCreate(K210Task, "K210", 128, NULL, 70, &g_xK210Handle);
		xTaskCreate(RC522Task,"RC522", 256, NULL, 65, &g_xRC522Handle);
		xTaskCreate(AS608Task,"AS608", 128, NULL, 68, &g_xAs608Handle);
		xTaskCreate(LightTask,"Light",56,NULL,62,&g_xLightHandle);
//	}

	vTaskStartScheduler();
   while (1)//启动成功，将不会执行该处
   {
	  
   }
}
