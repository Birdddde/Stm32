#include "OLEDFont.h"

#ifndef __OLED_H__
#define __OLED_H__
	
void OLED_Init(void);
void OLED_Clear(void);
void OLED_ClearArea(uint8_t Line, uint8_t Column, uint8_t Width, uint8_t Height);
void OLED_Update(void);
void OLED_ShowImage(uint8_t Line, uint8_t Column, uint8_t Width, uint8_t Height, const uint8_t (*Image)[Width]);
void OLED_ShowChar(uint8_t Line,uint8_t Column,uint8_t Char,uint8_t FontSize);
void OLED_ShowString(uint8_t Line ,uint8_t Column,char* String,uint8_t FontSize);

void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Num, uint8_t Length,uint8_t FontSize);
void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint8_t HexNum,uint8_t FontSize);
void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint8_t BinNum,uint8_t FontSize);
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Num, uint8_t Length,uint8_t FontSize);
void OLED_ShowFloatNum(uint8_t Line, uint8_t Column,double Number, uint8_t IntLength, uint8_t FraLength, uint8_t FontSize);
void OLED_ShowChinese(uint8_t Line, uint8_t Column,const char* Chinese);

#endif
