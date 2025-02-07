#include "stm32f10x.h"
#include <stdio.h>

#include "freertos.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

#include "oled.h"
#include "delay.h"
#include "usart.h"
#include "key.h"
#include "typedef.h"
#include "driver_rc522.h"
#include "menu.h"

#define MENU_MAIN_LENGTH 3

TaskHandle_t g_xRC522Handle=NULL;

void MenuTask(void *p){
	// 菜单初始化
    Menu_Init();
    Display_Refresh();
	
	while(1){
		uint8_t key = KeyNum_Get();
	   if(key != KEY_NONE) {
			Key_Handler(key);
	   }
	}
	
}

void RC522Task(void *p){
	uint8_t ucaID[4];
	uint8_t ucaCard_defaultkey [6] = {0xff,0xff,0xff,0xff,0xff,0xff};//默认密码
	uint8_t ucaCard_key [6] = {0xa1,0xa2,0xa3,0xa4,0xa5,0xa6};//密码
	uint8_t ucaBuffer_wr[16] = {0};//读写数据缓存
	uint8_t ucaControlB[16] = {0xa1,0xa2,0xa3,0xa4,0xa5,0xa6,		 //KEYA
									   0xff,0x07,0x80,0x69,					 //存取控制		
									   0x01,0x02,0x03,0x04,0x05,0x06};	 //KEYB
	uint8_t ucaVertify[16] = {0xfa,0xfb,0xfc,0xfd,0xfe,0xff,		 
									0x00,0x00,0x00,0x01,					 
									0x00,0x00,0x00,0x00,0x00,0x00};	 
	uint8_t ucStatus,ucErrorFlag;
	
	OLED_Clear();
	OLED_ShowString(1, 1, "Access Control", OLED_8X16);
	OLED_ShowString(1, 16, "System", OLED_8X16);
   OLED_Update();
	while(1){
		
		uint8_t cur_key = KeyNum_Get();
		if (cur_key == 3) {
			 OLED_Clear(); 
			 OLED_ShowString(1,1,"Register",OLED_6X8);
			 OLED_UpdateArea(1,1,128,32);
			 while(1){
				ucStatus = RC522_Test(PICC_AUTHENT1A,7,ucaID,ucaCard_defaultkey);
				if(ucStatus){
									
					ucStatus = 0;
					ucErrorFlag = 0;
					
					printf("CardID:%x%x%x%x\r\n",ucaID[0],ucaID[1],ucaID[2],ucaID[3]);
					
				   if( PcdWrite(7,ucaControlB) != MI_OK ){    			//写入控制块7
						printf("PcdWrite failure\r\n");          
						ucErrorFlag = 1;
					}
					else{ printf("PcdWrite ControlBlock success\r\n"); }						
				  
				   //写入数据块6
				   if( PcdWrite(6,ucaVertify) != MI_OK ){    
						printf("PcdWrite failure\r\n"); 
						ucErrorFlag = 1;		
					}else
				   { printf("PcdWrite DataBlock success\r\n"); }
					
					// 重新认证以确保控制块生效
					if (!ucErrorFlag) {
						 ucStatus = RC522_Test(PICC_AUTHENT1A,7,ucaID,ucaCard_key); // 使用新密钥认证
						 if (!ucStatus) {
							  printf("Re-Auth Fail. Check Control Block!\r\n");
							  ucErrorFlag = 1;
						 }
					}
					
					if (!ucErrorFlag) {
						printf("Register success!\r\n");
						break;
					}
					
				}
			 }
			
		}
		
		ucStatus = RC522_Test(PICC_AUTHENT1A,7,ucaID,ucaCard_key);
		if(ucStatus){
		  printf("CardID:%x%x%x%x\r\n",ucaID[0],ucaID[1],ucaID[2],ucaID[3]);				
		  ucStatus = 0;
		  
		  //读取数据块7的数据
		  if( PcdRead(7,ucaBuffer_wr) != MI_OK )
		  {    printf("PcdRead failure\r\n");           }
		  else
		  {
			  //输出读出的数据
			  printf("block 7:");
			  for( uint8_t i = 0; i < 16; i++ )
			  {
				  printf("%x",ucaBuffer_wr[i]);
			  }
			  printf("\r\n");
		  }
		  
		  //读取数据块6的数据
		  if( PcdRead(6,ucaBuffer_wr) != MI_OK )
		  {    printf("PcdRead failure\r\n");           }
		  else
		  {
				printf("block 6:");
			  //输出读出的数据
			  for( uint8_t i = 0; i < 16; i++ )
			  {
				  printf("%x",ucaBuffer_wr[i]);
			  }
			  printf("\r\n");		 
			}
		}
	}
}

int main(void)
{
	Serial_Init();
	OLED_Init();
	RC522_Init();
   Key_Init();  
	
	KeyScanTimer_Create();
	xTaskCreate(MenuTask, "Menu", 128, NULL, 60, &g_xRC522Handle);
//	xTaskCreate(RC522Task, "RC522", 128, NULL, 60, &g_xRC522Handle);
   vTaskStartScheduler();
}
