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

#define MENU_MAIN_LENGTH 3

void RC522Task(void *p);

TaskHandle_t g_xRC522Handle=NULL;
TaskHandle_t g_xMenuHandle=NULL;

// 菜单项声明
MenuItem Menu1, Menu2, Menu3;
MenuItem* mainMenu = &Menu1;
uint8_t currentMenuLevel = 0;

// 显示相关函数
void DisplayMainMenu() {
    OLED_Clear();
    MenuItem* current = mainMenu;
    for (int i = 0; i < MENU_MAIN_LENGTH; i++) {
        OLED_ShowString(1, 1 + i*9, current->label, OLED_6X8);
        current = current->next;
    }
	 
	 OLED_ShowString(1, 64-16, "<1>+ <2>- <3>Confirm", OLED_6X8);
    OLED_Update();
}

void HighlightMenuItem(uint8_t index ,uint8_t* last_index) {
    // 清除旧高亮
    
    if(last_index != 0){
        OLED_ReverseArea(1, (*last_index)*9 -8, 128, 8);
    }
    
    // 绘制新高亮
    OLED_ReverseArea(1, index*9 -8, 128, 8);
	 OLED_ShowNum(1,48,*last_index,2,OLED_6X8);
    *last_index = index;	   
	 OLED_Update();
}

// 菜单动作函数
void RegisrAction(void) {
    OLED_Clear();
    OLED_ShowString(1, 1, "Password:", OLED_6X8);
    OLED_Update();
}

void SetupAction(void) {
    OLED_Clear();
    OLED_ShowString(1, 1, "Set Password", OLED_6X8);
    OLED_Update();
}

void ExitAction(void) {
	 OLED_Clear();
	 OLED_Update();
	 vTaskResume(g_xRC522Handle);
	 vTaskSuspend(g_xMenuHandle);
}


void Menu_Init(void) {
    // 初始化菜单项
    Menu1 = (MenuItem){"1.Register", RegisrAction, &Menu2, &Menu3};
    Menu2 = (MenuItem){"2.Setup", SetupAction, &Menu3, &Menu1};
    Menu3 = (MenuItem){"3.Exit", ExitAction, &Menu1, &Menu2};
}

// 菜单任务
void MenuTask(void *pvParameters) {
    uint8_t index = 1;
	 static uint8_t last_index = 0;
    static uint8_t last_key = 0;
    
    Menu_Init();
    DisplayMainMenu();
	 HighlightMenuItem(index,&last_index);
	
    while(1) {
        uint8_t cur_key = KeyNum_Get();

        // 处理导航
        if(cur_key == 1 ) { // 上键
            index = (index == 1 && last_key != 1) ? MENU_MAIN_LENGTH : index-1;
            HighlightMenuItem(index,&last_index);
        }
        else if(cur_key == 2 && last_key != 2) { // 下键
            index = (index % MENU_MAIN_LENGTH) + 1;
            HighlightMenuItem(index,&last_index);
        }
        else if(cur_key == 3) { // 确认键
			   last_index = 0;
            MenuItem* current = mainMenu;
            for(int i = 1; i < index; i++) current = current->next;
            if(current->action) current->action();
        }
		  
			OLED_ShowNum(1,32,index,2,OLED_6X8);
		   OLED_Update();
		  
        last_key = cur_key;
    }
}

void RC522Task(void *p){
	uint8_t ucaID[4];
	uint8_t ucaCard_key [6] = {0xa1,0xa2,0xa3,0xa4,0xa5,0xa6};//默认密码
	uint8_t ucaBuffer_wr[16] = {0};//读写数据缓存
	uint8_t ucaControlB[16] = {0xa1,0xa2,0xa3,0xa4,0xa5,0xa6,		 //KEYA
									   0xff,0x07,0x80,0x69,					 //存取控制		
									   0x01,0x02,0x03,0x04,0x05,0x06};	 //KEYB
	uint8_t ucaVertify[16] = {0xfa,0xfb,0xfc,0xfd,0xfe,0xff,		 
									0x00,0x00,0x00,0x01,					 
									0x00,0x00,0x00,0x00,0x00,0x00};	 
	uint8_t ucStatus;
	
	OLED_Clear();
	OLED_ShowString(1, 1, "Access Control", OLED_8X16);
	OLED_ShowString(1, 16, "System", OLED_8X16);
   OLED_Update();
	while(1){
		
		uint8_t cur_key = KeyNum_Get();
		if (cur_key == 1) {
			 xTaskCreate(MenuTask, "Menu", 256, NULL, 64, &g_xMenuHandle);
			 vTaskSuspend(NULL);
		}
		
		ucStatus = RC522_Test(PICC_AUTHENT1A,7,ucaID,ucaCard_key);
		if(ucStatus){
		  printf("CardID:%x%x%x%x\r\n",ucaID[0],ucaID[1],ucaID[2],ucaID[3]);				
		  ucStatus = 0;
		  
			//写入数据块7
		  if( PcdWrite(7,ucaControlB) != MI_OK )
		  {    printf("PcdWrite failure\r\n");          }
		  
		  //写入数据块6
		  if( PcdWrite(6,ucaVertify) != MI_OK )
		  {    printf("PcdWrite failure\r\n");          }
		  
		  //读取数据块7的数据
		  if( PcdRead(7,ucaBuffer_wr) != MI_OK )
		  {    printf("PcdRead failure\r\n");           }

		  //输出读出的数据
		  printf("block 7:");
		  for( uint8_t i = 0; i < 16; i++ )
		  {
			  printf("%x",ucaBuffer_wr[i]);
		  }
		  printf("\r\n");
		  
		  //读取数据块6的数据
		  if( PcdRead(6,ucaBuffer_wr) != MI_OK )
		  {    printf("PcdRead failure\r\n");           }
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

int main(void)
{
	Serial_Init();
	OLED_Init();
	RC522_Init();
   Key_Init();  
	
	KeyScanTimer_Create();
	
	xTaskCreate(RC522Task, "RC522", 128, NULL, 60, &g_xRC522Handle);
   vTaskStartScheduler();
}
