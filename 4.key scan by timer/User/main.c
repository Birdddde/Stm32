#include "stm32f10x.h"
#include <stdio.h>
#include "freertos.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "oled.h"
#include "delay.h"
#include "key.h"

#define MENU_MAIN_LENGTH 2

uint8_t Pow(uint8_t base, uint8_t powerRaised);

void Menu(void *pvParameters)
{
	 // 创建定时器
   KeyScanTimer_Create();
	
	static uint8_t last_index = 0;
	static uint8_t cur_key_value = 0;
	static uint8_t index = 0;
	
   while(1)
   {
       cur_key_value = KeyNum_Get();
		
       if(cur_key_value == 1)
		 {			 
			index++;
			if(index > MENU_MAIN_LENGTH)	index = 1;
			 
			if(last_index != 0){
				OLED_ReverseArea(1,1 * Pow(10,last_index - 1),128,8);
				OLED_UpdateArea(1,1 *  Pow(10,last_index - 1),128,8); 
			} 
			 
			OLED_ReverseArea(1,1 * Pow(10,index - 1),128,8);
			OLED_UpdateArea(1,1 *  Pow(10,index - 1),128,8); 
		
			last_index = index;			
       }
		 
		 if(cur_key_value == 2)
		 {			 
			if(index == 1){
				OLED_Clear();
				OLED_ShowString(1,1,"Push Card!",OLED_6X8);
				OLED_Update();
			}
			if(index == 2){
				OLED_Clear();
				OLED_ShowString(1,1,"Vertifing...",OLED_6X8);
				OLED_Update();
			}
       }
		 		 
       vTaskDelay(10);
   }
}

int main(void)
{
	OLED_Init();
   Key_Init();  // 初始化GPIO按键
	
	OLED_ShowString(1,1," 1:Register",OLED_6X8);
	OLED_ShowString(1,10," 2:Vertify",OLED_6X8);
	OLED_UpdateArea(1,1,128,16); 

	//创建按键扫描任务
   xTaskCreate(Menu, "Menu", 256, NULL, 64, NULL);

    // 启动任务调度器
   vTaskStartScheduler();
}

uint8_t Pow(uint8_t base, uint8_t powerRaised)
{
    if (powerRaised != 0)
        return (base * Pow(base, powerRaised-1));
    else 
        return 1;
}
