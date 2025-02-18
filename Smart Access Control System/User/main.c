#include "stm32f10x.h"
#include <stdio.h>

#include "freertos.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "semphr.h"  

#include "oled.h"
#include "delay.h"
#include "uart1.h"
#include "uart2.h"
#include "key.h"
#include "typedef.h"
#include "rfid.h"
#include "menu.h"
#include "finger.h"

#define MENU_MAIN_LENGTH 3

TaskHandle_t g_xRC522Handle = NULL;
TaskHandle_t g_xMenuHandle = NULL;

volatile MenuState_t menuState = MENU_INACTIVE;
extern QueueHandle_t xQueueUart2;


void AS608Task(void *p){
	as608_status_t as608_status;
	uint8_t status = 0;

	while(1){
		status = KeyNum_Get();
		if(status){
			uint8_t state = Finger_Flush(&as608_status,NULL,NULL);
			if( state == FLUSH_FINGER_SUCCESS ){				
				OLED_ShowImage(48,16,32,32,Unlock32x32);
				OLED_Update();
				status = 0;
			}else if(state == 0){
				AS608_SendCommand(0x01,2,0x01,NULL);
				OLED_ShowString(0,0,"Receive outtime",OLED_8X16);
				OLED_Update();	
				status = 0;
			}else{
				OLED_ShowHexNum(0,0,as608_status,2,OLED_8X16);
				OLED_Update();	
				status = 0;
			}
//			Finger_Remove();
//			uint8_t state = Finger_Register(&as608_status,2);
//			if(state == 0){
//				AS608_SendCommand(0x01,2,0x01,NULL);
//				OLED_ShowString(0,0,"Receive outtime",OLED_8X16);
//				OLED_Update();	
//				status = 0;
//			}else if(state != REG_FINGER_SUCCESS){
//				OLED_ShowHexNum(0,0,as608_status,2,OLED_8X16);
//				OLED_Update();	
//				status = 0;
//			}
			
		}
	}

}

void MenuTask(void *p){
	OLED_ShowImage(48,16,32,32,Lock32x32);
	OLED_Update();
	while(1){
		
		if(xTaskNotifyWait(0, 0x01, NULL, 0) == pdPASS){
			 Menu_Init();
			 menuState = MENU_ACTIVE;
			 Display_Refresh();
			 OLED_ShowString(0,48,"<1>up       <2>down",OLED_6X8);
			 OLED_ShowString(0,56,"<3>confirm  <4>back",OLED_6X8);
			 OLED_Update();
		}		
		
		if(menuState == MENU_ACTIVE){
			uint8_t key = KeyNum_Get();
			if(key != KEY_NONE)	Key_Handler(key);

		   vTaskDelay(pdMS_TO_TICKS(10));
		}else{
			
		}
		
		vTaskDelay(pdMS_TO_TICKS(100)); // 非激活状态时降低检测频
	}
	
}

void RC522Task(void *p){
	
	uint8_t ucStatus,ucCur_key,ucFlag_admin = 0;
	uint32_t ulNotificationValue;
	
	while(1){
		
		ucCur_key = KeyNum_Get();
		ucStatus = RFID_Scan();
		
		if(ucCur_key == 3 && ucFlag_admin) {
			if(menuState == MENU_INACTIVE){
				 ucFlag_admin = 0;
				 xTaskNotify(g_xMenuHandle, 0x01, eSetBits);
				 vTaskSuspend(NULL);
			}
		}

		if(xTaskNotifyWait(0, 0x03, &ulNotificationValue, 0) == pdPASS) {
            if (ulNotificationValue & 0x01) {
                printf("Register");
                while( !RFID_Register() ){
						if (KeyNum_Get() == 4) {
							printf("Exit");
							break;
						}
					 }
                vTaskSuspend(NULL);
            }
            if (ulNotificationValue & 0x02) {
                printf("Remove");
                while( !RFID_Remove() ){
						if (KeyNum_Get() == 4) {
							printf("Exit");
							break;
						}
					 }
                vTaskSuspend(NULL);
            }
      }	
		
		if(ucStatus){
			RFID_ReadBlock(7);
			RFID_ReadBlock(6);
			OLED_ShowImage(48,16,32,32,Unlock32x32);
			OLED_Update();
			vTaskDelay(pdMS_TO_TICKS(1000));
			ucFlag_admin = 1;
		}
		vTaskDelay(pdMS_TO_TICKS(100)); // 非激活状态时降低检测频
	}
}

int main(void)
{
	Serial1_Init();
	OLED_Init();
	RFID_Init();
   Key_Init();  
	AS608_Init();
	
	KeyScanTimer_Create();
	
	xTaskCreate(MenuTask, "Menu", 128, NULL, 60, &g_xMenuHandle);
	xTaskCreate(RC522Task, "RC522", 128, NULL, 64, &g_xRC522Handle);
	xTaskCreate(AS608Task, "AS608", 128, NULL, 65, &g_xRC522Handle);
	
   vTaskStartScheduler();
}
