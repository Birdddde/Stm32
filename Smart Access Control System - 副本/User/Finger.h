#include "driver_as608.h"

#ifndef __FINGER_H
#define __FINGER_H

void Finger_Init(void);
uint8_t PS_StaIO(void);
uint8_t Finger_Flush(as608_status_t* as608_status,uint16_t * Page_id,uint16_t * Match_score);
uint8_t Finger_Register(as608_status_t* as608_status,uint16_t Page_id);
uint8_t Finger_Remove_Handler(void);
void Finger_Regis_Handler(void);

#endif
