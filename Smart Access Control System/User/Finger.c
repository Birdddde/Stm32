#include "stm32f10x.h"                  // Device header
#include "finger.h"
#include "oled.h"
#include "key.h"

#include "freertos.h"
#include "task.h"
#include "queue.h"

extern TaskHandle_t g_xAs608Handle;
uint16_t g_usFingerId;


void Finger_Init(void){
	AS608_Init();
	
	EXTI_InitTypeDef EXTI_InitStruc;
	EXTI_InitStruc.EXTI_Line = EXTI_Line1;
	EXTI_InitStruc.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStruc.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStruc.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStruc);
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	
	NVIC_InitTypeDef NVIC_InitStruc;
	NVIC_InitStruc.NVIC_IRQChannel = EXTI1_IRQn;
	NVIC_InitStruc.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStruc.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStruc.NVIC_IRQChannelSubPriority = 10;
	
	NVIC_Init(&NVIC_InitStruc);
	
}
uint8_t PS_StaIO(void){
	return GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_1);
}



uint8_t Finger_Flush(as608_status_t* as608_status,uint16_t * Page_id,uint16_t * Match_score){
	as608_status_t status;
	
	
	// 获取指纹图像
	if (!PS_GetImage(&status)) {
	   return 0;
	}
	if (status != AS608_STATUS_OK) {
	   *as608_status = status;
	   return FLUSH_FINGER_GET_IMAGE_FAILED;
	}

	// 生成特征字符
	if (!PS_GenChar(&status, AS608_BUFFER_NUMBER_1)) {
	  return 0;
	}
	if (status != AS608_STATUS_OK) {
	   *as608_status = status;
	   return FLUSH_FINGER_GEN_CHAR_FAILED;
	}

	// 搜索指纹库
	if (!PS_Search(&status,Page_id,Match_score, AS608_BUFFER_NUMBER_1, 0, 300)) {
	  return 0;
	}
	if (status != AS608_STATUS_OK) {
		*as608_status = status;
	   return FLUSH_FINGER_SEARCH_FAILED;
	}

	// 显示指纹 id
	return FLUSH_FINGER_SUCCESS;
}

uint8_t Finger_Register(as608_status_t* as608_status, uint16_t Page_id) {
    as608_status_t status;

    // 第一次获取图像并生成到BUFFER_1
    if (!PS_GetImage(&status) || status != AS608_STATUS_OK) {
        *as608_status = status;
        return status == AS608_STATUS_OK ? 0 : REG_FINGER_GET_IMAGE_FAILED;
    }

    if (!PS_GenChar(&status, AS608_BUFFER_NUMBER_1) || status != AS608_STATUS_OK) {
        *as608_status = status;
        return REG_FINGER_GEN_CHAR_FAILED;
    }

    // 第二次获取图像并生成到BUFFER_2
    if (!PS_GetImage(&status) || status != AS608_STATUS_OK) {
        *as608_status = status;
        return REG_FINGER_GET_IMAGE_FAILED;
    }

    if (!PS_GenChar(&status, AS608_BUFFER_NUMBER_2) || status != AS608_STATUS_OK) {
        *as608_status = status;
        return REG_FINGER_GEN_CHAR_FAILED;
    }

    // 合并特征并存储
    if (!PS_RegModel(&status) || status != AS608_STATUS_OK) {
        *as608_status = status;
        return REG_FINGER_REG_MODEL_FAILED;
    }

    if (!PS_StoreChar(&status, AS608_BUFFER_NUMBER_1, Page_id) || status != AS608_STATUS_OK) {
        *as608_status = status;
        return REG_FINGER_STORE_CHAR_FAILED;
    }

    return REG_FINGER_SUCCESS;
}

uint8_t Finger_Remove_Handler(void){
	as608_status_t status;
	uint16_t Page_id,Match_score;
	
	uint8_t fun_state = Finger_Flush(&status,&Page_id,&Match_score);
	
	switch( fun_state ){
		case 0:
			OLED_ShowString(0,32,"Overtime",OLED_6X8);
			OLED_UpdateArea(0,32,128,16);
			break;
		case FLUSH_FINGER_SUCCESS:
			OLED_ShowString(0,32,"Finger Matched",OLED_6X8);
			OLED_ShowString(0,40,"ID:",OLED_6X8);	
			OLED_ShowNum(5*6,40,Page_id,2,OLED_6X8);
			OLED_UpdateArea(0,32,128,16);
		
			if( !PS_DeletChar(&status,Page_id,1) ){
				OLED_ShowString(0,32,"Overtime",OLED_6X8);
				OLED_UpdateArea(0,32,128,16);
			}			
			if(status != AS608_STATUS_OK){
				OLED_ShowString(0,32,"Delete Failed",OLED_6X8);
				OLED_UpdateArea(0,32,128,16);
			}else{
				OLED_ShowString(0,32,"Remove Finished",OLED_6X8);
				OLED_UpdateArea(0,32,128,16);
				return 1;			//success
			}			
			break;
		case FLUSH_FINGER_GET_IMAGE_FAILED:
			if(status == AS608_STATUS_NO_FINGERPRINT){
				OLED_ShowString(0,32,"NO FINGERPRINT",OLED_6X8);
				OLED_UpdateArea(0,32,128,16);
			}else{			
				OLED_ShowString(0,32,"GET IMAGE FAILED",OLED_6X8);
				OLED_ShowString(0,40,"erro code:",OLED_6X8);
				OLED_ShowNum(6*11,40,status,2,OLED_6X8);
				OLED_UpdateArea(0,32,128,16);
			}
			break;
		case FLUSH_FINGER_GEN_CHAR_FAILED:
			OLED_ShowString(0,32,"GEN CHAR FAILED",OLED_6X8);
			OLED_ShowString(0,40,"erro code:",OLED_6X8);
			OLED_ShowNum(6*11,40,status,2,OLED_6X8);
			OLED_UpdateArea(0,32,128,16);
			break;

		case FLUSH_FINGER_SEARCH_FAILED:
			if(status == AS608_STATUS_NOT_FOUND){
				OLED_ShowString(0,32,"Finger not found",OLED_6X8);
				OLED_UpdateArea(0,32,128,16);
			}else{
				OLED_ShowString(0,32,"SEARCH FAILED",OLED_6X8);
				OLED_ShowString(0,40,"erro code:",OLED_6X8);
				OLED_ShowNum(6*11,40,status,2,OLED_6X8);
				OLED_UpdateArea(0,32,128,16);
			}
			break;
	}
	return 0;	//failed
}

void Finger_Regis_Handler(void){
	as608_status_t as608_status;
	uint8_t status;
	
	OLED_ClearArea(0,24,128,24);
	OLED_ShowString(0,32,"scanning...",OLED_6X8);
	OLED_UpdateArea(0,24,128,24);
	while(1){
		
		if (KeyNum_Get() == 4) 
		{
			OLED_ClearArea(0,24,128,24);
			OLED_UpdateArea(0,24,128,24);
			break;
		}
		
		if(g_usFingerId >300){					//是否超过存储容量范围
			OLED_ClearArea(0,24,128,24);
			OLED_ShowString(0,32,"Finger data is full",OLED_6X8);
			OLED_UpdateArea(0,24,128,24);
			vTaskDelay(2000);
			OLED_ClearArea(0,24,128,24);
			OLED_UpdateArea(0,24,128,24);
			break;
		}else
		{
			status = Finger_Register(&as608_status,g_usFingerId++);

			if( status == REG_FINGER_SUCCESS ){
				OLED_ClearArea(0,24,128,24);
				OLED_ShowString(0,32,"Register success !",OLED_6X8);
				OLED_UpdateArea(0,24,128,24);
				vTaskDelay(2000);
				break;
			}else{
				OLED_ClearArea(0,24,128,24);
				OLED_ShowString(0,32,"Registering...",OLED_6X8);
				OLED_UpdateArea(0,24,128,24);
				vTaskDelay(2000);
			}
			
		}		
	}
	OLED_ClearArea(0,24,128,24);
	OLED_UpdateArea(0,24,128,24);
	
}

void EXTI1_IRQHandler(void){
	
	if (EXTI_GetITStatus(EXTI_Line1))
	{
		xTaskNotify(g_xAs608Handle, 0x01, eSetBits);
	}
	EXTI_ClearITPendingBit(EXTI_Line1);
	
}
