#ifndef __RFID_H
#define __RFID_H

uint8_t RFID_Init(void);
uint8_t RFID_Register(void);
uint8_t RFID_Scan(void);
void RFID_ReadBlock(uint8_t Block);
uint8_t RFID_Remove(void);

#endif
