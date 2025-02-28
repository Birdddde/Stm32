#include "stm32f10x.h"
#include <stdio.h>
#include "freertos.h"
#include "task.h"
#include "queue.h"

#include "uart1.h"
#include "esp01s_wifimoudle.h"

TaskHandle_t LedHandle=NULL;
uint8_t code;

void MQTTTask(void *p){

	
//	if(code != 1){
//		printf("%u",code);
//	}
//	if(  ){
//		printf("Connect error:%u",code);	
//	}
	MQTT_UploadState(1);
	while (1) {

	}
	
}


int main(void)
{
	USART1_Init(115200);
	DMA1_Init();
	Esp01s_ConnectAli(&code);
	
	xTaskCreate(MQTTTask,"MQTTTask",56,NULL,2,&LedHandle);
	
	vTaskStartScheduler();
	
  while (1)//启动成功，将不会执行该处
  {
	  
  }
}

