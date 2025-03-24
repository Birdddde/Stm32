#include "stm32f10x.h"                  // Device header
#include "uart3.h"
#include "string.h"
#include "key.h"
#include "oled.h"

uint8_t rx_flag;

uint8_t Face_Scan(void){

		uint8_t* data = Uart3_GetData(&rx_flag);
			
		if(rx_flag == 1)
		{
			rx_flag = 0;
			if(strncmp((const char*)data, "feature matched", 14) == 0)
			{
				return 1;
			}				
		}
		return 0;
}

void Face_Regis_Handler(void){

	Serial3_SendString("@ADDCOMMON2");
	OLED_ShowString(0,24,"Registering",OLED_6X8);
	OLED_UpdateArea(0,24,128,16);
	while(1){
		uint8_t* data = Uart3_GetData(&rx_flag);
		if (KeyNum_Get() == 4) 
		{
			OLED_ClearArea(0,24,128,24);
			OLED_UpdateArea(0,24,128,24);
			break;
		}			
		if(rx_flag == 1)
		{
			if(strncmp((const char*)data, "stor successful", 14) == 0)
			{
				OLED_ClearArea(0,24,128,24);
				OLED_ShowString(0,24,"Face Reg success",OLED_6X8);
				OLED_UpdateArea(0,24,128,24);
			}				
			else
			{
				OLED_ClearArea(0,24,128,24);
				OLED_ShowString(0,24,"Face Reg failed",OLED_6X8);
				OLED_UpdateArea(0,24,128,24);
			}
			vTaskDelay(pdMS_TO_TICKS(2000));
			OLED_ClearArea(0,24,128,24);
			OLED_UpdateArea(0,24,128,24);
			rx_flag = 0;
			break;
		}
	}
}

void Face_Remove_Handler(void){
	Serial3_SendString("@DELETECUR");
	OLED_ShowString(0,24,"Removing",OLED_6X8);
	OLED_UpdateArea(0,24,128,16);
	while(1){
		uint8_t* data = Uart3_GetData(&rx_flag);
		if (KeyNum_Get() == 4) 
		{
			OLED_ClearArea(0,24,128,24);
			OLED_UpdateArea(0,24,128,24);
			break;
		}			
		if(rx_flag == 1)
		{
			if(strncmp((const char*)data, "feature deleted", 14) == 0)
			{
				OLED_ClearArea(0,24,128,24);
				OLED_ShowString(0,24,"Face Remove success",OLED_6X8);
				OLED_UpdateArea(0,24,128,24);
			}				
			else
			{
				OLED_ClearArea(0,24,128,24);
				OLED_ShowString(0,24,"Face Remove failed",OLED_6X8);
				OLED_UpdateArea(0,24,128,24);
			}
			vTaskDelay(pdMS_TO_TICKS(2000));
			OLED_ClearArea(0,24,128,24);
			OLED_UpdateArea(0,24,128,24);
			rx_flag = 0;
			break;
		}
	}

}

void Face_Init(void){
	rx_flag = 0;
	Serial3_Init(115200);//k210
}

