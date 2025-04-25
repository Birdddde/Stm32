#include "stm32f10x.h"                  // Device header
#include <stdio.h>
#include "string.h"
#include "driver_rc522.h"  
#include "uart1.h" 
#include "oled.h"
#include "key.h"
#include "storage.h"

// 控制码
uint8_t ucaControlB[16] = {0xa1,0xa2,0xa3,0xa4,0xa5,0xa6,		 
									0xff,0x07,0x80,0x69,					 
									0x01,0x02,0x03,0x04,0x05,0x06};	

// 验证码
uint8_t ucaVertify[16] = {0xfa,0xfb,0xfc,0xfd,0xfe,0xff,		 
								0x00,0x00,0x00,0x01,					 
								0x00,0x00,0x00,0x00,0x00,0x00};	

uint8_t ucaBuffer_wr[16] = {0};									//读写数据缓存
uint8_t ucaCard_key[6] = {0xa1,0xa2,0xa3,0xa4,0xa5,0xa6};	    //密码
uint8_t ucaCard_defaultkey[6] = {0xff,0xff,0xff,0xff,0xff,0xff};//默认密码
uint8_t ucaID[4];												//卡ID

/**
  * @brief  初始化RFID
  * @param  无
  * @retval 无
  */
void RFID_Init(void){
	RC522_Init();
}

/**
  * @brief  读取块
  * @param  块号
  * @retval 无
  */
void RFID_ReadBlock(uint8_t Block){
	  
  if( PcdRead(Block,ucaBuffer_wr) != MI_OK )
  {    
//	  printf("PcdRead Block %x failure\r\n",Block);  
  }
  else
  {
//	  printf("block :%x: ",Block);
//	  for( uint8_t i = 0; i < 16; i++ )
//	  {
//		  printf("%x",ucaBuffer_wr[i]);
//	  }
//	  printf("\r\n");
  }
}

/**
  * @brief  测试RFID
  * @param  认证模式,块号,卡ID,密码
  * @retval 1:成功,0:失败
  */
uint8_t RFID_Test(uint8_t ucAuth_mode, uint8_t ucAddr,uint8_t* pArray_ID,uint8_t* pCard_Key){
	
	if(PcdRequest ( PICC_REQALL, pArray_ID ) == MI_OK)	//寻卡
	{		
		if ( PcdAnticoll ( pArray_ID ) == MI_OK )			//防冲突机制
		{    	  
			if( PcdSelect(pArray_ID) == MI_OK )				//选卡
			{
			  if( PcdAuthState(ucAuth_mode,ucAddr,pCard_Key,pArray_ID) == MI_OK )	//三次相互验证
					return 1;
		  }
		}
	}
	return 0;
}

/**
  * @brief  扫描RFID
  * @param  卡ID
  * @retval 1:成功,0:失败
  */
uint8_t RFID_Scan(uint8_t* CardID){
	uint16_t card_cnt;
	
	if( RFID_Test(PICC_AUTHENT1A,7,ucaID,ucaCard_key) ){
		
		for(uint8_t i = 0 ; i < 4 ;i ++)
		{
			CardID[i] = ucaID[i];
		}
//		if( RC522_ID_IsExist(ucaID,&card_cnt) )	
		return 1;
	}
	return 0;
}

/**
  * @brief  注册RFID
  * @param  无
  * @retval 1:成功,0:失败
  */
uint8_t RFID_Register(void){

	if( RFID_Test(PICC_AUTHENT1A,7,ucaID,ucaCard_defaultkey) ){					
	  
		if( PcdWrite(6,ucaVertify) != MI_OK ){    
			return 0;
		}

		if( PcdWrite(7,ucaControlB) != MI_OK ){    			
			return 0;
		}		
		return 1;
	}
	return 0;
}

/**
  * @brief  移除RFID
  * @param  无
  * @retval 1:成功,0:失败
  */
uint8_t RFID_Remove(void){
	uint16_t cnt;
	
   for( uint8_t i = 0; i < 6; i++ )
   { 
	   ucaBuffer_wr[i] = ucaCard_defaultkey[i];
   }
   for( uint8_t i = 7-1; i < 10; i++ )
   { 
	   ucaBuffer_wr[i] = ucaControlB[i];
   }
	for( uint8_t i = 10; i < 16; i++ )
   { 
	   ucaBuffer_wr[i] = ucaCard_defaultkey[i-10];
   }
	if( RFID_Scan(ucaID) ){					
				
		if( PcdWrite(7,ucaBuffer_wr) != MI_OK )	return 0;
	  
		memset(ucaBuffer_wr,0,sizeof(ucaBuffer_wr));
		
		if( PcdWrite(6,ucaBuffer_wr) != MI_OK )	return 0;

//		if(RC522_Delete(cnt) != FLASH_OK)	return 0;	//删除Flash内卡数据
		
		return 1;
	}
	return 0;
}

/**
  * @brief  注册RFID
  * @param  无
  * @retval 无
  */
void RFID_Regis_Handler(void){
	uint8_t status;

	while( 1 ){
		if (KeyNum_Get() == 4) {
			OLED_ClearArea(40,24,128-40,24);
			OLED_UpdateArea(40,24,128-40,24);
			break;
		}
		status = RFID_Register();
		if( status ){
//			if(RC522_WriteIDToFlash(ucaID) == FLASH_OK){
				OLED_ClearArea(40,24,128,24);
				OLED_ShowString(40,32,"Register success !",OLED_6X8);
				OLED_UpdateArea(40,24,128,24);

				vTaskDelay(2000);
				OLED_ClearArea(40,24,128,24);
				OLED_UpdateArea(40,24,128,24);
				break;
//			}
		}else{
			OLED_Clear();
			OLED_ShowChinese(40,32,"注册卡");
			OLED_Update();
		}
	}
}

/**
  * @brief  移除RFID
  * @param  无
  * @retval 无
  */
void RFID_Remove_Handler(void){
	
	while( 1 )
	{
		if (KeyNum_Get() == 4) 
		{
			OLED_ClearArea(40,24,128-40,24);
			OLED_UpdateArea(40,24,128-40,24);
			break;
		}						
		if( RFID_Remove() )
		{
			OLED_ClearArea(40,24,128,24);
			OLED_ShowString(40,32,"Remove success !",OLED_6X8);
			OLED_UpdateArea(40,24,128,24);
			vTaskDelay(2000);
			OLED_ClearArea(40,24,128,24);
			OLED_UpdateArea(40,24,128,24);
			break;
		}else{
			OLED_Clear();
			OLED_ShowChinese(40,32,"移除卡");
			OLED_Update();
		}
	}
}
