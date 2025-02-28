#include "stm32f10x.h"
#include <stdio.h>

#include "freertos.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"  

#include "oled.h"
#include "delay.h"
#include "uart1.h"
#include "uart2.h"
#include "key.h"
#include "tim.h"
#include "typedef.h"
#include "rfid.h"
#include "menu.h"
#include "finger.h"
#include "servo.h"
#include "beep.h"
#include "esp01s_wifimoudle.h"

#define MENU_MAIN_LENGTH 3

TaskHandle_t g_xRC522Handle = NULL;
TaskHandle_t g_xMenuHandle = NULL;
TaskHandle_t g_xAs608Handle = NULL;
uint8_t g_ucCode;
uint8_t g_ucDoor_status = 0;

volatile MenuState_t menuState = MENU_INACTIVE;
extern QueueHandle_t xQueueUart2;
uint8_t Admin_pass[4] = {0x00,0x00,0x00,0x00};
uint16_t Page_id=0;

void AS608Task(void *p){
	as608_status_t as608_status;
	uint8_t status = 0;
	uint32_t ulNotificationValue;
	
	while(1){
		if(xTaskNotifyWait(0, 0x03, &ulNotificationValue, 0) == pdPASS) {
			OLED_ClearArea(0,24,128,24);
			OLED_ShowString(0,32,"Registering...",OLED_6X8);
			OLED_UpdateArea(0,24,128,24);
			if(ulNotificationValue & 0x01){
				if(Page_id >300){
					OLED_ClearArea(0,24,128,24);
					OLED_ShowString(0,32,"Finger data is full",OLED_6X8);
					OLED_UpdateArea(0,24,128,24);
					vTaskDelay(2000);
					OLED_ClearArea(0,24,128,24);
					OLED_UpdateArea(0,24,128,24);
				}else{
					status = Finger_Register(&as608_status,Page_id++);

					if( status == REG_FINGER_SUCCESS ){
						OLED_ClearArea(0,24,128,24);
						OLED_ShowString(0,32,"Register success !",OLED_6X8);
						OLED_UpdateArea(0,24,128,24);
						vTaskDelay(2000);
						OLED_ClearArea(0,24,128,24);
						OLED_UpdateArea(0,24,128,24);
					}else{
						OLED_ClearArea(0,24,128,24);
						OLED_ShowString(0,32,"Register failed !",OLED_6X8);
						OLED_UpdateArea(0,24,128,24);
						vTaskDelay(2000);
						OLED_ClearArea(0,24,128,24);
						OLED_UpdateArea(0,24,128,24);
					}
				}
				
				vTaskSuspend(NULL);
			}
			
			if(ulNotificationValue & 0x02){
				status = Finger_Remove();

				vTaskSuspend(NULL);
			}
		}
				
		if(PS_StaIO()){			
			status = Finger_Flush(&as608_status,NULL,NULL);					
			if( status == FLUSH_FINGER_SUCCESS ){				
				xTaskNotify(g_xMenuHandle,0x02,eSetBits);
			}else{
				Beep_On(1000);
			}
		}
	
	}

}

void MenuTask(void *p){
	uint8_t key;

	uint32_t ulNotificationValue;
	uint8_t Flag_Access = 0;
	
	while(1){
		
		if(xTaskNotifyWait(0, 0x03, &ulNotificationValue, 0) == pdPASS) {
			if(ulNotificationValue & 0x01){
				vTaskSuspend(g_xRC522Handle);	//暂停任务
				vTaskSuspend(g_xAs608Handle);
				Menu_Init();
				menuState = MENU_ACTIVE;
				Display_Refresh();
			}
			if(ulNotificationValue & 0x02){
				Flag_Access = 1;
			}
		}
		
		
		if(menuState == MENU_ACTIVE){
			key = KeyNum_Get();
			if(key != KEY_NONE)	Key_Handler(key);

		   vTaskDelay(pdMS_TO_TICKS(10));
		}else{
			if(Flag_Access){
				OLED_ShowImage(48,16,32,32,Unlock32x32);
				Servo_SetAngle(180);
				g_ucDoor_status = 1;
				vTaskDelay(pdMS_TO_TICKS(5000));
				Flag_Access=0;
			}else{
				OLED_ShowImage(48,16,32,32,Lock32x32);
				Servo_SetAngle(0);
				g_ucDoor_status = 0;
			}			
			OLED_UpdateArea(48,16,32,32);
			
			key = KeyNum_Get();
			if(key){	
				Pass_handlle(Admin_pass,Action_COMAPRE);
			}
		}
		
		vTaskDelay(pdMS_TO_TICKS(100)); // 非激活状态时降低检测频
	}
	
}

void RC522Task(void *p){
	
	uint8_t ucStatus;
	uint32_t ulNotificationValue;
	
	while(1){

		ucStatus = RFID_Scan();
		
		if(xTaskNotifyWait(0, 0x03, &ulNotificationValue, 0) == pdPASS) {
			if (ulNotificationValue & 0x01) {
				
				 while( 1 ){
					if (KeyNum_Get() == 4) {	
						break;
					}
					if( RFID_Register() ){
						OLED_ClearArea(0,24,128,24);
						OLED_ShowString(0,32,"Register success !",OLED_6X8);
						OLED_UpdateArea(0,24,128,24);
						vTaskDelay(2000);
						OLED_ClearArea(0,24,128,24);
						OLED_UpdateArea(0,24,128,24);
						break;
					}
						
				 }
				 vTaskSuspend(NULL);
			}
			if (ulNotificationValue & 0x02) {

				 while( 1 ){
					if (KeyNum_Get() == 4) {
						printf("Exit");
						break;
					}						
					if( RFID_Remove() ){
						OLED_ClearArea(0,24,128,24);
						OLED_ShowString(0,32,"Remove success !",OLED_6X8);
						OLED_UpdateArea(0,24,128,24);
						vTaskDelay(2000);
						OLED_ClearArea(0,24,128,24);
						OLED_UpdateArea(0,24,128,24);
						break;
					}
				 }
				 vTaskSuspend(NULL);
			}
      }	
		
		if(ucStatus){
			RFID_ReadBlock(7);
			RFID_ReadBlock(6);
			xTaskNotify(g_xMenuHandle,0x02,eSetBits);
			vTaskDelay(pdMS_TO_TICKS(1000));
		}
		vTaskDelay(pdMS_TO_TICKS(100)); // 非激活状态时降低检测频
	}
}

int main(void)
{
	Serial1_Init(115200);
	OLED_Init();
	RFID_Init();
   Key_Init();  
	Finger_Init();
	Beep_Init();
	Servo_Init();
		
	DMA1_Init();
	Esp01s_ConnectAli(&g_ucCode);
	
	Timer_Create();
	
	xTaskCreate(MenuTask, "Menu", 128, NULL, 60, &g_xMenuHandle);
	xTaskCreate(RC522Task, "RC522", 128, NULL, 64, &g_xRC522Handle);
	xTaskCreate(AS608Task, "AS608", 128, NULL, 65, &g_xAs608Handle);
	
   vTaskStartScheduler();
}
