#ifndef __MENU_H
#define __MENU_H

void Display_Refresh(void);
void Menu_Init(void);
void Key_Handler(uint8_t key);
void Pass_handlle(uint8_t* Password,Menu_Pass_Action_t Action);

#endif
