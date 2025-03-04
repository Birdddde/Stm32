#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "USART.h"
#include "ESP01S_WIFIMOUDLE.h"
#include "Key.h"
#include "String.h"

uint8_t KeyNum;
uint8_t AT_TestFlag=0;
int main(void)
{
	OLED_Init();
	Serial_Init();
	Key_Init();
	OLED_ShowString(1,1,"SS");
	while (1)
	{
		
		if(!AT_TestFlag){
		   AT_TestFlag=Esp01s_TestAT();
			if(!AT_TestFlag) {
				OLED_ShowString(4,1,"           ");
				OLED_ShowString(4,1, "Testing...");
			}else{
				OLED_ShowString(4,1,"           ");
				OLED_ShowString(4,1, "Ready!");
			}
		}
		
		KeyNum=Key_GetNum();
		if(KeyNum == 1 && AT_TestFlag==1){
			Esp01s_SendCommand("RST",0,0);
			Delay_ms(1000);
			Esp01s_SetCWMODE();
			Delay_ms(1000);
			Esp01s_SetSNTPServer();
			Delay_ms(1000);			
			Esp01s_ConnectWifi();
			Delay_ms(3000);
			Esp01s_ConnectMQTT();
			Delay_ms(2000);
			MQTT_Subscribe();
			Delay_ms(2000);
			MQTT_UploadNon(44);
		}
		
		if(Serial_GetRxFlag() == 1)
		{
//			if( strstr(Receive,"OK") ){
//				OLED_ShowString(4,1,"      ");
//				OLED_ShowString(4,1, "Ready!");
//				AT_TestFlag=1;
//			}
		}
		
	}
}
