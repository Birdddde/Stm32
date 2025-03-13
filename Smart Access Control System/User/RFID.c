#include "stm32f10x.h"                  // Device header
#include <stdio.h>
#include "string.h"
#include "driver_rc522.h"  
#include "uart1.h" 
#include "oled.h"
#include "key.h"

uint8_t ucaControlB[16] = {0xa1,0xa2,0xa3,0xa4,0xa5,0xa6,		 
									0xff,0x07,0x80,0x69,					 
									0x01,0x02,0x03,0x04,0x05,0x06};	

uint8_t ucaVertify[16] = {0xfa,0xfb,0xfc,0xfd,0xfe,0xff,		 
								0x00,0x00,0x00,0x01,					 
								0x00,0x00,0x00,0x00,0x00,0x00};	

uint8_t ucaBuffer_wr[16] = {0};											 //读写数据缓存
uint8_t ucaCard_key[6] = {0xa1,0xa2,0xa3,0xa4,0xa5,0xa6};	    //密码
uint8_t ucaCard_defaultkey[6] = {0xff,0xff,0xff,0xff,0xff,0xff};//默认密码
uint8_t ucaID[4];

void RFID_Init(void){
	RC522_Init();
}

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

uint8_t RFID_Scan(void){
	
	if( RFID_Test(PICC_AUTHENT1A,7,ucaID,ucaCard_key) ){
		return 1;
	}
	return 0;
}

void RFID_ReadBlock(uint8_t Block){
	  
  if( PcdRead(Block,ucaBuffer_wr) != MI_OK )
  {    printf("PcdRead Block %x failure\r\n",Block);  }
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

uint8_t RFID_Register(void){

	if( RFID_Test(PICC_AUTHENT1A,7,ucaID,ucaCard_defaultkey) ){					

		if( PcdWrite(7,ucaControlB) != MI_OK ){    			
//			printf("PcdWrite failure\r\n");          
			return 0;
		}
//		else{ printf("PcdWrite ControlBlock success\r\n"); }						
	  
		if( PcdWrite(6,ucaVertify) != MI_OK ){    
//			printf("PcdWrite failure\r\n"); 
			return 0;
		}
//		else
//		{ printf("PcdWrite DataBlock success\r\n"); }
		 
		 return 1;
	}
	return 0;
}

uint8_t RFID_Remove(void){
	
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
	if( RFID_Scan() ){					

		if( PcdWrite(7,ucaBuffer_wr) != MI_OK ){    			
//			printf("PcdWrite failure\r\n");          
			return 0;
		}
//		else{ printf("PcdWrite ControlBlock success\r\n"); }						
	  
		memset(ucaBuffer_wr,0,sizeof(ucaBuffer_wr));
		if( PcdWrite(6,ucaBuffer_wr) != MI_OK ){    
//			printf("PcdWrite failure\r\n"); 
			return 0;
		}
//		else{ printf("PcdWrite DataBlock success\r\n"); }
		 
		 return 1;
	}
	return 0;
}

void RFID_Regis_Handler(void){
	uint8_t status;
//	OLED_ClearArea(0,24,128,24);
//	OLED_ShowString(0,32,"scanning...",OLED_6X8);
//	OLED_UpdateArea(0,24,128,24);
	while( 1 ){
		if (KeyNum_Get() == 4) {
			OLED_ClearArea(0,24,128,24);
			OLED_UpdateArea(0,24,128,24);
			break;
		}
		status = RFID_Register();
		if( status ){
			OLED_ClearArea(0,24,128,24);
			OLED_ShowString(0,32,"Register success !",OLED_6X8);
			OLED_UpdateArea(0,24,128,24);
			vTaskDelay(2000);
			OLED_ClearArea(0,24,128,24);
			OLED_UpdateArea(0,24,128,24);
			break;
		}else{
			OLED_ClearArea(0,24,128,24);
			OLED_ShowString(0,32,"Registering",OLED_6X8);
			OLED_UpdateArea(0,24,128,24);
		}
	}
}

void RFID_Remove_Handler(void){
	
	OLED_ClearArea(0,24,128,24);
	OLED_ShowString(0,32,"scanning...",OLED_6X8);
	OLED_UpdateArea(0,24,128,24);
	while( 1 )
	{
		if (KeyNum_Get() == 4) 
		{
			OLED_ClearArea(0,24,128,24);
			OLED_UpdateArea(0,24,128,24);
			break;
		}						
		if( RFID_Remove() )
		{
			OLED_ClearArea(0,24,128,24);
			OLED_ShowString(0,32,"Remove success !",OLED_6X8);
			OLED_UpdateArea(0,24,128,24);
			vTaskDelay(2000);
			OLED_ClearArea(0,24,128,24);
			OLED_UpdateArea(0,24,128,24);
			break;
		}else{
			OLED_ClearArea(0,24,128,24);
			OLED_ShowString(0,32,"Removing",OLED_6X8);
			OLED_UpdateArea(0,24,128,24);
		}
	}
}
