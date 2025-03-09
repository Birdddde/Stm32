#include "stm32f10x.h"
#include <stdio.h>
#include "freertos.h"
#include "task.h"
#include "queue.h"

#include "OLED.h"
#include "HCSR04.h"
#include "delay.h"

TaskHandle_t LedHandle=NULL;
	
void LEDTask(void *p){
	TickType_t Tick=pdMS_TO_TICKS(1000);
	float length;
//	Hcsr04Init();	
	
	while(1){
		
		length = sonar();
//		OLED_ShowFloatNum(0,0,length,3,2,OLED8X16);
//		OLED_Update();
//		vTaskDelay(Tick);

	}
	
}


int main(void)
{
	OLED_Init();
	
//	xTaskCreate(LEDTask,"LEDTask",56,NULL,2,&LedHandle);
	
	vTaskStartScheduler();
	
  while (1)//启动成功，将不会执行该处
  {
	  
  }
}

