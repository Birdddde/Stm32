#include "stm32f10x.h"                  // Device header
#include <stdio.h>
#include "key.h"
#include "delay.h"
#include "oled.h"
#include "typedef.h"
#include "freertos.h"
#include "esp01s_wifimoudle.h"
#include "finger.h"
#include "rfid.h"
#include "face.h"

extern TaskHandle_t g_xRC522Handle,g_xMenuHandle,g_xAs608Handle,g_xK210Handle;
extern volatile MenuState_t menuState;
extern uint8_t g_ucaAdmin_pass[4];

void Pass_handlle(uint8_t* Password,Menu_Pass_Action_t Action);

MenuItem* current_menu = NULL;  // 当前选中菜单
MenuItem* menu_root = NULL;     // 根菜单
Menu_Operation_t g_xMenu_Opera;
uint8_t display_offset = 0;     

// 声明所有菜单项（静态分配）
MenuItem menu_main,menu_exit, menu_settings, menu_system_info,menu_pass_set;
MenuItem menu_register,menu_remove,menu_rfid,menu_finger,menu_face;

void Display_Refresh(void) {
    OLED_Clear();
	 OLED_ShowString(0,48,"<1>up       <2>down",OLED_6X8);
	 OLED_ShowString(0,56,"<3>confirm  <4>back",OLED_6X8);

    MenuItem* p = current_menu->parent ? current_menu->parent->child : menu_root;
    
    // 显示同级菜单项（最多显示4项）
    for(int i=0; i<4 && p!=NULL; i++, p=p->next) {
        if(p == current_menu) {
            OLED_ShowString(0, i*8, ">", OLED_6X8);
            OLED_ShowString(6, i*8, p->name, OLED_6X8);
        } else {
            OLED_ShowString(6, i*8, p->name, OLED_6X8);
        }
    }
	 
    OLED_Update();
}

void Register_Func(void) {
	g_xMenu_Opera = OPERATION_REGISTER;
}

void Remove_Func(void) {
    g_xMenu_Opera = OPERATION_REMOVE;
}

void Settings_Func(void) {
    g_xMenu_Opera = OPERATION_NONE;
}

void Operation_RFID(void) {
    // 实际硬件操作 
	 switch (g_xMenu_Opera){
		 case OPERATION_REGISTER:
			RFID_Regis_Handler();
			break;
		 case OPERATION_REMOVE:
			RFID_Remove_Handler();
			break;
		 default:
			OLED_ShowString(0,32,"Error occured",OLED_6X8);
		   OLED_UpdateArea(0,32,128,16);
			break;
	 }
}

void Operation_Finger(void) {

	NVIC_DisableIRQ(EXTI1_IRQn);	//关闭指纹外部中断
	switch (g_xMenu_Opera){
		case OPERATION_REGISTER:
			Finger_Regis_Handler();
			break;
		case OPERATION_REMOVE:
			Finger_Remove_Handler();
			break;
		default:
			OLED_ShowString(0,32,"Error occured",OLED_6X8);
			OLED_UpdateArea(0,32,128,16);
			break;
		
	}
	NVIC_EnableIRQ(EXTI1_IRQn);
}

void Operation_Face(void) {
	
	switch (g_xMenu_Opera){
		case OPERATION_REGISTER:
			Face_Regis_Handler();
			break;
		case OPERATION_REMOVE:
			Face_Remove_Handler();
			break;
		default:
			OLED_ShowString(0,32,"Error occured",OLED_6X8);
			OLED_UpdateArea(0,32,128,16);
			break;
	}
	OLED_UpdateArea(0,32,128,16);
}

void Exit_Func(void) {
    OLED_Clear();
    OLED_Update();
    Delay_ms(1000);
	
	 menuState = MENU_INACTIVE;
	 vTaskResume(g_xRC522Handle);
	 vTaskResume(g_xAs608Handle);
	vTaskResume(g_xK210Handle);
}

void Pass_Func(){
	Pass_handlle(g_ucaAdmin_pass,Action_SET);
}

void Show_SystemInfo(void) {
	 OLED_ShowString(0,32,"version:1.0.0",OLED_6X8);
    OLED_UpdateArea(0,32,128,8);
    Delay_ms(1000);
}

// 初始化菜单结构
void Menu_Init(void) {
    // 主菜单
    menu_main.name = "Main Menu";
    menu_main.parent = NULL;
    menu_main.child = &menu_settings;
    menu_main.func = NULL;
	 menu_main.next = &menu_exit;
	 
	 menu_exit.name = "Exit";
    menu_exit.parent = NULL;
    menu_exit.child = NULL;
    menu_exit.func = Exit_Func;
	 menu_exit.prev = &menu_main;
    // 设置子菜单
    menu_settings.name = "Settings";
    menu_settings.parent = &menu_main;
    menu_settings.child = &menu_register;
    menu_settings.func = Settings_Func;
	 // 设置子菜单
    menu_register.name = "Register";
    menu_register.parent = &menu_settings;
    menu_register.child = &menu_rfid;
    menu_register.func = Register_Func;
	 menu_register.next = &menu_remove;
	
	 menu_remove.name = "Remove";
    menu_remove.parent = &menu_settings;
    menu_remove.child = &menu_rfid;
    menu_remove.func = Remove_Func;
	 menu_remove.next = &menu_pass_set;
	 menu_remove.prev = &menu_register;
	 
	 menu_pass_set.name = "Set admin password";
	 menu_pass_set.parent = &menu_settings;
	 menu_pass_set.child = NULL;
	 menu_pass_set.func = Pass_Func;
	 menu_pass_set.prev = &menu_remove;
	 
	 // 设置子菜单	 
	 menu_rfid.name = "RFID Card";
    menu_rfid.parent = &menu_register;
    menu_rfid.child = NULL;
    menu_rfid.func = Operation_RFID;
	 menu_rfid.next = &menu_finger;
	 
	 menu_finger.name = "Finger Print";
    menu_finger.parent = &menu_register;
    menu_finger.child = NULL;
    menu_finger.func = Operation_Finger;
	 menu_finger.next = &menu_face;
	 menu_finger.prev = &menu_rfid;
	 
	 menu_face.name = "Face";
    menu_face.parent = &menu_register;
	 menu_face.prev = &menu_finger;
	 menu_face.func = Operation_Face;
	 
    // 系统信息
    menu_system_info.name = "System Info";
    menu_system_info.parent = &menu_main;
    menu_system_info.child = NULL;
    menu_system_info.func = Show_SystemInfo;
    menu_settings.next = &menu_system_info;
    menu_system_info.prev = &menu_settings;

    // 初始化指针
    current_menu = &menu_main;
    menu_root = &menu_main;
}

void Key_Handler(uint8_t key) {
    switch(key) {
        case KEY_UP:
            if(current_menu->prev) {
                current_menu = current_menu->prev;
                Display_Refresh();
            }
            break;
            
        case KEY_DOWN:
            if(current_menu->next) {
                current_menu = current_menu->next;
                Display_Refresh();
            }
            break;
            
        case KEY_ENTER:
				if(current_menu->func) {  // 执行功能
                current_menu->func();
            }
				if(current_menu->child) {  // 进入子菜单
                current_menu = current_menu->child;
                Display_Refresh();
            }
            break;
            
        case KEY_BACK:
            if(current_menu->parent) {
                current_menu = current_menu->parent;
                Display_Refresh();
            }
            break;
    }
}

void Pass_handlle(uint8_t* Password,Menu_Pass_Action_t Action){
	uint8_t Pass[4]={0,0,0,0};;
	uint8_t Pass_index = 0,key=0;
	
	OLED_Clear();
	OLED_ShowString(0,8,"Password:",OLED_8X16);
	OLED_ShowString(0,64-8,"1:+ 2:- 3:Sel 4:OK",OLED_6X8);
	OLED_Update();
	
	while(key != 4){
		key = KeyNum_Get();
		if( key == 1 ){					
			Pass[Pass_index]++;
			Pass[Pass_index] %= 10;					
		}
		if( key == 2 ){	
			if(Pass[Pass_index] == 0) Pass[Pass_index]=10;
			Pass[Pass_index]--;	
			Pass[Pass_index] %= 10;																					
		}
		if( key == 3 ){	
			Pass_index ++;
			Pass_index %= 4;
		}
		
		OLED_Clear();
		OLED_ShowString(Pass_index*10+40,33,"_",OLED_8X16);
		OLED_ShowNum(40,32,Pass[0],1,OLED_8X16);
		OLED_ShowNum(50,32,Pass[1],1,OLED_8X16);
		OLED_ShowNum(60,32,Pass[2],1,OLED_8X16);
		OLED_ShowNum(70,32,Pass[3],1,OLED_8X16);
		OLED_UpdateArea(40,32,128,17);					
	}
	if(Action == Action_COMAPRE){
		if( Pass[0] == Password[0] && Pass[1] == Password[1] && Pass[2] == Password[2] && Pass[3] == Password[3] ){
			OLED_Clear();
			OLED_ShowString(0,0,"Pass correct",OLED_6X8);
			OLED_Update();
			vTaskDelay(pdMS_TO_TICKS(1000));
			xTaskNotify(g_xMenuHandle, 0x01, eSetBits);
		}else{
			OLED_Clear();
			OLED_ShowString(0,0,"Pass incorrect",OLED_6X8);
			OLED_Update();
			vTaskDelay(pdMS_TO_TICKS(1000));
		}	
	}
	if(Action == Action_SET){
		Password[0] = Pass[0];
		Password[1] = Pass[1];
		Password[2] = Pass[2];
		Password[3] = Pass[3];	
		OLED_Clear();
		OLED_ShowString(0,0,"Pass set accessed",OLED_6X8);
		OLED_Update();
//		MQTT_UploadPass(Pass);		//上传阿里云
		vTaskDelay(pdMS_TO_TICKS(1000));
		Display_Refresh();
	}

	 OLED_Clear();	
}

