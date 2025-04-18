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
#include "uart1.h"
#include "storage.h"
#include "delay.h"

#define MENU_MAIN_LENGTH 3

TaskHandle_t g_xRC522Handle = NULL;
TaskHandle_t g_xMenuHandle = NULL;
TaskHandle_t g_xAs608Handle = NULL;
TaskHandle_t g_xLightHandle = NULL;
TaskHandle_t g_xK210Handle = NULL;
TaskHandle_t g_xWifiHandle = NULL;

wifi_error_t g_xError = {0,NULL};	//阿里云连接错误标志位
uint8_t g_ucDoor_status = 0;			//门锁状态标志位

SemaphoreHandle_t g_xMutex_Wifi,g_xMutex_Key,g_xOledMutex;

volatile MenuState_t menuState = MENU_INACTIVE;
extern QueueHandle_t xQueueUart2;
uint8_t g_ucaAdmin_pass[4];
extern uint16_t g_usFingerId;

uint8_t g_uckey = 0;	//按键

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
#ifdef WIFI					
				MQTT_UploadState(1);
#endif
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
				vTaskSuspend(g_xWifiHandle);
				
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
			if(g_xError.error_code)	
				OLED_ShowImage(0,0,12,12,Wifi_Disconnected12x12);
			else
				OLED_ShowImage(0,0,12,12,Wifi_Connected12x12);
			
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
			OLED_Update();
			
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
				MQTT_UploadState(1);
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

void WifiTask(void *p){
	Uart_Rx_t xUart1_rx;
	while(1){
		
		if(USART1_GetRxData(&xUart1_rx)){
			xUart1_rx.rx_buffer[xUart1_rx.rx_data_length + 1] = '\0';
			if(strstr((const char*)xUart1_rx.rx_buffer,"@get_password"))
				MQTT_UploadPass(g_ucaAdmin_pass);
			if(strstr((const char*)xUart1_rx.rx_buffer,"@set_lockstate\":1"))
				xTaskNotify(g_xMenuHandle,0x02,eSetBits);
			if(strstr((const char*)xUart1_rx.rx_buffer,"@set_lockstate\":0")){						
				Servo_Control_Angle(0);
				g_ucDoor_status = 0;
			}
		
		}
		vTaskDelay( pdMS_TO_TICKS(500) );
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
	ESP01S_Init();
	Delay_ms(2000);	//等待硬件初始化完成 
	
	g_xMutex_Key = xSemaphoreCreateMutex();
	g_xOledMutex = xSemaphoreCreateMutex();
	g_xMutex_Wifi= xSemaphoreCreateMutex();	//创建互斥信号量	

#ifdef WIFI	
	OLED_ShowChinese(40,32-8,"连接中");
	OLED_Update();

	Esp01s_ConnectAli(&g_xError);
	OLED_Clear();	//清屏
	OLED_Update();
	MQTT_UploadPass(g_ucaAdmin_pass);
#endif

	Timer_Create();	//创建软件定时器
	
	if (g_xError.error_code)
	{		
		OLED_ShowString(0,0,"Connect Aliyun failed",OLED_6X8);
		OLED_ShowString(0,8,"error code:",OLED_6X8);
		OLED_ShowString(0,16,"error src:",OLED_6X8);
		OLED_ShowNum(13*6,8,g_xError.error_code,2,OLED_6X8);
		OLED_ShowNum(13*6,16,g_xError.error_src,2,OLED_6X8);
		OLED_ShowString(0,24,"press any key to exit",OLED_6X8);
		OLED_Update();
		
		Delay_ms(5000);
		OLED_Clear();	//清屏
	}		
	
	xTaskCreate(MenuTask, "Menu", 128, NULL, 60, &g_xMenuHandle);
	xTaskCreate(K210Task, "K210", 128, NULL, 63, &g_xK210Handle);
	xTaskCreate(RC522Task,"RC522", 256, NULL, 65, &g_xRC522Handle);
	xTaskCreate(AS608Task,"AS608", 128, NULL, 68, &g_xAs608Handle);
	xTaskCreate(LightTask,"Light",56,NULL,50,&g_xLightHandle);
	xTaskCreate(WifiTask,"Wifi",56,NULL,62,&g_xWifiHandle);
	

	vTaskStartScheduler();
   while (1)//启动成功，将不会执行该处
   {
	  
   }
}
