#include "stm32f10x.h"
#include <stdio.h>
#include "freertos.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "oled.h"
#include "delay.h"
#include "key.h"

void Key_Scan_Task(void *pvParameters)
{
   uint8_t key_value = 0;
   while(1)
   {
       key_value = KeyNum_Get();
       if(key_value != 0)
       {
           OLED_ShowNum(2,2,key_value,2);
       }
       vTaskDelay(10);
   }
}

int main(void)
{
	OLED_Init();
   Key_Init();  // 初始化GPIO按键
	
	OLED_ShowString(1,1,"Key:");
    // 创建定时器
   Timer_Create();
	//创建按键扫描任务
   xTaskCreate(Key_Scan_Task, "Key_Scan_Task", 256, NULL, 64, NULL);

    // 启动任务调度器
   vTaskStartScheduler();
}


