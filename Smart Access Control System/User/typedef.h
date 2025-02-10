#ifndef __TYPEDEF_H
#define __TYPEDEF_H
	
typedef void (*MenuFunction)(void); // 菜单功能回调函数类型

typedef struct MenuItem {
    char        *name;          // 菜单显示名称
    struct MenuItem *parent;    // 父菜单
    struct MenuItem *child;     // 子菜单
    struct MenuItem *prev;      // 前一个同级菜单
    struct MenuItem *next;      // 后一个同级菜单
    MenuFunction func;          // 功能函数指针
} MenuItem;

typedef enum {
    MENU_ACTIVE,
    MENU_INACTIVE
} MenuState_t;

#endif
