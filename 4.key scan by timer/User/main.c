#include "stm32f10x.h"
#include <stdio.h>

#include "freertos.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "semphr.h"  

#include "oled.h"
#include "delay.h"
#include "usart.h"
#include "key.h"
#include "typedef.h"
#include "rfid.h"
#include "menu.h"

#define MENU_MAIN_LENGTH 3

TaskHandle_t g_xRC522Handle = NULL;
TaskHandle_t g_xMenuHandle = NULL;
volatile MenuState_t menuState = MENU_INACTIVE;

void MenuTask(void *p){

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
		}
		vTaskDelay(pdMS_TO_TICKS(100)); // 非激活状态时降低检测频
	}
	
}

void RC522Task(void *p){
	
	uint8_t ucStatus;
	uint32_t ulNotificationValue;
	
	while(1){
		
		uint8_t cur_key = KeyNum_Get();
		if (cur_key == 3) {
			if(menuState == MENU_INACTIVE){
				 xTaskNotify(g_xMenuHandle, 0x01, eSetBits);
				 vTaskSuspend(NULL);
			}
		}
		
		if(xTaskNotifyWait(0, 0x03, &ulNotificationValue, 0) == pdPASS) {
            if (ulNotificationValue & 0x01) {
                printf("Register");
                while(!RFID_Register());
                vTaskSuspend(NULL);
            }
            if (ulNotificationValue & 0x02) {
                printf("Remove");
                while(!RFID_Remove());
                vTaskSuspend(NULL);
            }
      }	
		
		ucStatus = RFID_Scan();
		if(ucStatus){
			RFID_ReadBlock(7);
			RFID_ReadBlock(6);
		}
		
	}
}

int main(void)
{
	Serial_Init();
	OLED_Init();
	RFID_Init();
   Key_Init();  
		
	KeyScanTimer_Create();
	
	xTaskCreate(MenuTask, "Menu", 128, NULL, 60, &g_xMenuHandle);
	xTaskCreate(RC522Task, "RC522", 128, NULL, 64, &g_xRC522Handle);
	
   vTaskStartScheduler();
}
