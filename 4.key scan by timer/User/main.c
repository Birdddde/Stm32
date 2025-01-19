#include "stm32f10x.h"
#include <stdio.h>
#include "freertos.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "oled.h"
#include "delay.h"
#include "key.h"

	 

int main(void)
{
	OLED_Init();
   Key_Init();  // 初始化GPIO按键
	
	OLED_ShowString(1,1,"helo");
    // 创建定时器
   Timer_Create();
	
    // 启动任务调度器
   vTaskStartScheduler();
}


