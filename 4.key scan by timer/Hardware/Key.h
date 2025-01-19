#ifndef __KEY_H
#define __KEY_H
	
	#include "freertos.h"
	#include "timers.h"
		
	void Key_Init(void);
	void Timer_Create(void);
	uint8_t KeyNum_Get(void);

#endif
