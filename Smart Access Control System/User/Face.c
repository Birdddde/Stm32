#include "stm32f10x.h"                  // Device header
#include "uart3.h"
#include "string.h"
#include "key.h"
#include "oled.h"

uint8_t rx_flag;

/**
  * @brief  扫描人脸
  * @param  无
  * @retval 1:成功 0:失败
  */
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

/**
  * @brief  注册人脸
  * @param  无
  * @retval 无
  */
void Face_Regis_Handler(void){

	Serial3_SendString("@ADDCOMMON2");
	OLED_Clear();
	OLED_ShowChinese(40,32,"注册人脸");
	OLED_Update();
	while(1){
		uint8_t* data = Uart3_GetData(&rx_flag);
		if (KeyNum_Get() == 4) 
		{
			OLED_ClearArea(40,24,128-40,24);
			OLED_UpdateArea(40,24,128-40,24);
			break;
		}			
		if(rx_flag == 1)
		{
			if(strncmp((const char*)data, "stor successful", 14) == 0)
			{
				OLED_ClearArea(40,24,128,24);
				OLED_ShowString(40,24,"Face Reg success",OLED_6X8);
				OLED_UpdateArea(40,24,128,24);
			}				
			else
			{
				OLED_ClearArea(40,24,128,24);
				OLED_ShowString(40,24,"Face Reg failed",OLED_6X8);
				OLED_UpdateArea(40,24,128,24);
			}
			vTaskDelay(pdMS_TO_TICKS(2000));
			OLED_ClearArea(40,24,128,24);
			OLED_UpdateArea(40,24,128,24);
			rx_flag = 0;
			break;
		}
	}
}

/**
  * @brief  移除人脸
  * @param  无
  * @retval 无
  */
void Face_Remove_Handler(void){
	Serial3_SendString("@DELETECUR");
	OLED_Clear();
	OLED_ShowChinese(40,32,"移除人脸");
	OLED_Update();
	vTaskDelay(pdMS_TO_TICKS(1500));
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

/**
  * @brief  初始化
  * @param  无
  * @retval 无
  */
void Face_Init(void){
	rx_flag = 0;
	Serial3_Init(115200);//k210
}

