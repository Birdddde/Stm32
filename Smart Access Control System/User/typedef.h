#ifndef __TYPEDEF_H
#define __TYPEDEF_H
/*
	菜单结构体
*/	
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

typedef enum {
	OPERATION_NONE = 0,
	OPERATION_REGISTER,
	OPERATION_REMOVE
} Menu_Operation_t;

/*
	AS608串口结构体
*/	
typedef enum {
    STATE_WAIT_HEADER1,  // 等待包头第1字节
    STATE_WAIT_HEADER2,  // 等待包头第2字节
    STATE_READ_LENGTH,   // 读取长度字段
    STATE_READ_DATA,     // 读取数据内容
    STATE_READ_CheckSum  // 读取校验和
} UartRxState;

typedef struct {
    UartRxState state;      // 当前状态
    uint8_t	*buffer;        // 动态缓冲区指针
    uint16_t expected_len;   // 期望接收的总长度（header+length+data+crc）
    uint16_t received_len;   // 已接收的字节数
    uint16_t data_len;       // 数据部分长度
} UartReceiver;



#endif
