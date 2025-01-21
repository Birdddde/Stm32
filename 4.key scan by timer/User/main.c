#include "stm32f10x.h"
#include <stdio.h>
#include "freertos.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "oled.h"
#include "delay.h"
#include "key.h"
#include "typedef.h"


#define MENU_MAIN_LENGTH 3

void DisplayMenu(MenuItem *menu);

static uint8_t last_index = 0;
static uint8_t cur_key_value = 0;
static uint8_t index = 0;

void MMenuAction(void)
{
	OLED_ShowString(1,1,"1.Register",OLED_6X8);
	OLED_ShowString(1,10,"2.Setup",OLED_6X8);
	OLED_ShowString(1,19,"3.Exit",OLED_6X8);
	
	OLED_UpdateArea(1,1,128,32); 
}
void RegisrAction(void)
{
	OLED_Clear();
	OLED_ShowString(1,1," Password:",OLED_6X8);
	OLED_UpdateArea(1,1,128,32); 
}

void SetupAction(void)
{
	OLED_Clear();
	OLED_ShowString(1,1,"Set Password",OLED_6X8);
	OLED_UpdateArea(1,1,128,32); 
}

MenuItem Menu1 = {"MainMenu",&MMenuAction,NULL,NULL};
MenuItem Menu2 = {"Setup",&SetupAction,NULL,NULL};
MenuItem Menu3 = {"Register",&RegisrAction,NULL,NULL};
MenuItem *mainMenu = &Menu1;	
	                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         
void DisplayMenu(MenuItem *menu) {
    if (menu != NULL) {
		  char* label = (char*)(menu->label);
        OLED_ShowString(1,48,label,OLED_6X8);
		  OLED_UpdateArea(1,48,128,8);
        menu = menu->next;
    }
}


void Menu_Init(void)
{
	Menu1.next = &Menu2;
	Menu2.next = &Menu3;
}


void Menu(void *pvParameters)
{
	 // 创建定时器
   KeyScanTimer_Create();
		
	DisplayMenu(mainMenu);
	mainMenu ->action();				
	while(1)
   {
       cur_key_value = KeyNum_Get();
		
       if(cur_key_value == 1)
		 {			 
			index++;
			if(index > MENU_MAIN_LENGTH)	index = 1;
			 
			if(last_index != 0){
				OLED_ReverseArea(1,last_index * 9 - 8,128,8);
				OLED_UpdateArea(1,last_index * 9 - 8,128,8); 
			} 
			 
			OLED_ReverseArea(1,index*9 -8,128,8);
			OLED_UpdateArea(1,index*9 -8,128,8); 
		
			last_index = index;			
       }
		 
		 if(cur_key_value == 2)
		 {			 
			if(index == 1){
				index = 0;
				(mainMenu -> next) ->next -> action();
				DisplayMenu((mainMenu -> next) ->next);
			}
			if(index == 2){
				index = 0;
				(mainMenu -> next) -> action();
				DisplayMenu(mainMenu -> next );
			}
			if(index == 3){
				index = 0;
				OLED_Clear();
				OLED_ShowString(1,1,"Access Control",OLED_8X16);
				OLED_ShowString(1,1 * 16,"System",OLED_8X16);
				OLED_Update();
			}
       }
 		 OLED_ShowNum(1,40,index,2,OLED_6X8);
		 OLED_UpdateArea(1,40,128,8); 

       vTaskDelay(10);
   }
}

int main(void)
{
	OLED_Init();
   Key_Init();  // 初始化GPIO按键
	Menu_Init();

	//创建按键扫描任务
   xTaskCreate(Menu, "Menu", 256, NULL, 64, NULL);

    // 启动任务调度器
   vTaskStartScheduler();
}
