#ifndef __TYPEDEF_H
#define __TYPEDEF_H
	
typedef struct MenuItem {
    const char *label;               // 菜单项名称
    void (*action)(void);            // 菜单项对应的操作函数
    struct MenuItem *next;           // 指向下一个菜单项的指针
    struct MenuItem *submenu;        // 指向子菜单的指针
} MenuItem;

#endif
