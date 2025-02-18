#include "stm32f10x.h"                  // Device header
#include "finger.h"
#include "oled.h"

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

uint8_t Finger_Remove(void){
	as608_status_t status;
	uint16_t Page_id,Match_score;
	
	uint8_t fun_state = Finger_Flush(&status,&Page_id,&Match_score);
	
	switch( fun_state ){
		case 0:
			OLED_ShowString(0,0,"Overtime",OLED_6X8);
			OLED_UpdateArea(0,0,128,16);
			break;
		case FLUSH_FINGER_SUCCESS:
			OLED_ShowString(0,0,"Finger Matched",OLED_6X8);
			OLED_ShowString(0,8,"Page:",OLED_6X8);
			OLED_ShowString(9*6,8,"Score:",OLED_6X8);		
			OLED_ShowNum(6*6,8,Page_id,2,OLED_6X8);
			OLED_ShowNum(15*6,8,Match_score,2,OLED_6X8);
			OLED_UpdateArea(0,0,128,16);
		
			if( !PS_DeletChar(&status,Page_id,1) ){
				OLED_ShowString(0,0,"Overtime",OLED_6X8);
				OLED_UpdateArea(0,0,128,16);
			}			
			if(status != AS608_STATUS_OK){
				OLED_ShowString(0,0,"Delete Failed",OLED_6X8);
				OLED_UpdateArea(0,0,128,16);
			}else{
				OLED_ShowString(0,0,"Remove Finished",OLED_6X8);
				OLED_UpdateArea(0,0,128,16);
				return 1;			//success
			}			
			break;
		case FLUSH_FINGER_GET_IMAGE_FAILED:
			if(status == AS608_STATUS_NO_FINGERPRINT){
				OLED_ShowString(0,0,"NO FINGERPRINT",OLED_6X8);
				OLED_UpdateArea(0,0,128,16);
			}else{			
				OLED_ShowString(0,0,"GET IMAGE FAILED",OLED_6X8);
				OLED_ShowString(0,16,"erro code:",OLED_6X8);
				OLED_ShowNum(6*11,16,status,2,OLED_6X8);
				OLED_UpdateArea(0,0,128,32);
			}
			break;
		case FLUSH_FINGER_GEN_CHAR_FAILED:
			OLED_ShowString(0,0,"GEN CHAR FAILED",OLED_6X8);
			OLED_ShowString(0,16,"erro code:",OLED_6X8);
			OLED_ShowNum(11,16,status,2,OLED_6X8);
			OLED_UpdateArea(0,0,128,32);
			break;

		case FLUSH_FINGER_SEARCH_FAILED:
			if(status == AS608_STATUS_NOT_FOUND){
				OLED_ShowString(0,0,"Finger not found",OLED_6X8);
				OLED_UpdateArea(0,0,128,16);
			}else{
				OLED_ShowString(0,0,"SEARCH FAILED",OLED_6X8);
				OLED_ShowString(0,16,"erro code:",OLED_6X8);
				OLED_ShowNum(11,16,status,2,OLED_6X8);
				OLED_UpdateArea(0,0,128,32);
			}
			break;
	}
	return 0;	//failed
}

