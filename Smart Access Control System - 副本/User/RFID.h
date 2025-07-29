#ifndef __RFID_H
#define __RFID_H

uint8_t RFID_Init(void);
uint8_t RFID_Register(void);
uint8_t RFID_Scan(uint8_t* CardID);
void RFID_ReadBlock(uint8_t Block);
uint8_t RFID_Remove(void);
void RFID_Regis_Handler(void);
void RFID_Remove_Handler(void);
#endif
