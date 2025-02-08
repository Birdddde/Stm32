#include "stm32f10x.h"
#include <stdio.h>

#include "freertos.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

#include "oled.h"
#include "delay.h"
#include "usart.h"
#include "key.h"
#include "typedef.h"
#include "rfid.h"
#include "menu.h"

#define MENU_MAIN_LENGTH 3

TaskHandle_t g_xRC522Handle=NULL;

void MenuTask(void *p){
	// 菜单初始化
    Menu_Init();
    Display_Refresh();
	
	while(1){
		uint8_t key = KeyNum_Get();
	   if(key != KEY_NONE) {
			Key_Handler(key);
	   }
	}
	
}

void RC522Task(void *p){
	
	uint8_t ucStatus;
	
	OLED_Clear();
	OLED_ShowString(1, 1, "Access Control", OLED_8X16);
	OLED_ShowString(1, 16, "System", OLED_8X16);
   OLED_Update();
	while(1){
		
		uint8_t cur_key = KeyNum_Get();
		if (cur_key == 3) {
			 OLED_Clear(); 
			 OLED_ShowString(1,1,"Register",OLED_6X8);
			 OLED_UpdateArea(1,1,128,32);
			
			 while(!RFID_Register()){
				
			 }			
//			 while(!RFID_Remove()){
//				
//			 }
			 RFID_ReadBlock(7);
			 RFID_ReadBlock(6);
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
//	xTaskCreate(MenuTask, "Menu", 128, NULL, 60, &g_xRC522Handle);
	xTaskCreate(RC522Task, "RC522", 128, NULL, 62, &g_xRC522Handle);
   vTaskStartScheduler();
}
