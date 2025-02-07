#include "freertos.h"
#include "timers.h"

#ifndef __KEY_H
#define __KEY_H
	
	#define KEY_NONE 0	
	#define KEY_UP 1
	#define KEY_DOWN 2
	#define KEY_ENTER 3
	#define KEY_BACK 4
	
	
	void Key_Init(void);
	void KeyScanTimer_Create(void);
	uint8_t KeyNum_Get(void);

#endif
