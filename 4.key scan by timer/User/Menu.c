#include "stm32f10x.h"                  // Device header
#include "key.h"
#include "delay.h"
#include "oled.h"
#include "typedef.h"

#define OPERATION_NONE 0
#define OPERATION_REGISTER 1
#define OPERATION_REMOVE 2

MenuItem* current_menu = NULL;  // 当前选中菜单
MenuItem* menu_root = NULL;     // 根菜单
uint8_t display_offset = 0;     // 显示偏移量（用于长列表）

// 声明所有菜单项（静态分配）
MenuItem menu_main, menu_settings, menu_system_info;
MenuItem menu_register,menu_remove,menu_rfid,menu_finger,menu_face;
uint8_t g_ucOperation;

void Display_Refresh(void) {
    OLED_Clear();
    
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
	 
    OLED_UpdateArea(0,0,128,32);
}

void Register_Func(void) {
    g_ucOperation = OPERATION_REGISTER;
}

void Remove_Func(void) {
    g_ucOperation = OPERATION_REMOVE;
}

void Settings_Func(void) {
    g_ucOperation = OPERATION_NONE;
}

void Operation_Func(void) {
    // 实际硬件操作
	 OLED_ShowNum(0,32,g_ucOperation,1,OLED_6X8);
    OLED_UpdateArea(0,32,128,8);
    Delay_ms(1000);
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

    // 设置子菜单
    menu_settings.name = "Settings";
    menu_settings.parent = &menu_main;
    menu_settings.child = &menu_register;
    menu_settings.func = Settings_Func;

    menu_register.name = "Register";
    menu_register.parent = &menu_settings;
    menu_register.child = &menu_rfid;
    menu_register.func = Register_Func;
	 menu_register.next = &menu_remove;
	
	 menu_remove.name = "Remove";
    menu_remove.parent = &menu_settings;
    menu_remove.child = &menu_rfid;
    menu_remove.func = Remove_Func;
	 menu_remove.prev = &menu_register;
	 
	 menu_rfid.name = "RFID Card";
    menu_rfid.parent = &menu_register;
    menu_rfid.child = NULL;
    menu_rfid.func = Operation_Func;
	 menu_rfid.next = &menu_finger;
	 
	 menu_finger.name = "Finger Print";
    menu_finger.parent = &menu_register;
    menu_finger.child = NULL;
    menu_finger.func = Operation_Func;
	 menu_finger.next = &menu_face;
	 menu_finger.prev = &menu_rfid;
	 
	 menu_face.name = "Face";
    menu_face.parent = &menu_register;
	 menu_face.prev = &menu_finger;
	 menu_face.func = Operation_Func;
	 
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
	 
	 OLED_ShowString(0,48,"<1>up       <2>down",OLED_6X8);
	 OLED_ShowString(0,56,"<3>confirm  <4>back",OLED_6X8);
	 OLED_Update();
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


