#ifndef __OLEDFont_H__
#define __OLEDFont_H__

#include "String.h"

//*宏定义																																																*//
#define MAX_CHN_CHARLEN 3				//一个中文字所占字节大小，取决于编码方式(UTF-8为3；GB2312为2)

#define OLED6X8 6	              //字模大小，新增高度时需修改OLED_ShowChar函数
#define OLED8X16 8

//中文结构体，包含中文字与对应字模数据
typedef struct
{
	char* Chinese;
	uint8_t Ch_Data[2][16];
}Chinese_t;

//字模存储
extern const uint8_t Ascii_F6x8[][6];
extern const uint8_t Ascii_F8x16[][8*2];
extern const uint8_t Image16x16[][16*2];
extern const Chinese_t Chinese16x16[];




#endif
