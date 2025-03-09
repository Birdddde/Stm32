#include "stm32f10x.h"                  // Device header
#include "OLED_I2C.h"
#include "DELAY.h"
#include "OLEDFont.h"
#include <math.h>
#include "freertos.h"
#include "task.h"

#define OLED_MAX_LINES 64-1
#define OLED_MAX_COLUMNS 128-1

//初始化显示缓冲区
uint8_t OLED_SHOWBUFFER[8][128]={0};

void OLED_ClearArea(uint8_t Line, uint8_t Column, uint8_t Width, uint8_t Height);

/**
	* @brief  发送命令至SSD1306
	* @param  Command:需要写入的命令
  * @retval 无
  */
void OLED_WriteCommand(uint8_t Command){
	OLED_I2C_Start();
	OLED_I2C_SendByte(0x78); //Slaver address
	OLED_I2C_ReceiveACK();
	OLED_I2C_SendByte(0x00);	//Write command
	OLED_I2C_ReceiveACK();
	OLED_I2C_SendByte(Command);
	OLED_I2C_ReceiveACK();
	OLED_I2C_Stop();
}

/**
	* @brief  发送数据至SSD1306
	* @param  Data:需要写入的数据
  * @retval 无
  */
void OLED_WriteData(uint8_t* Data,uint8_t Length)
{
	OLED_I2C_Start();
	OLED_I2C_SendByte(0x78); //Slaver address
	OLED_I2C_ReceiveACK();
	OLED_I2C_SendByte(0x40);	//Write command
	OLED_I2C_ReceiveACK();
	for(uint8_t i=0;i<Length;i++){
		OLED_I2C_SendByte(Data[i]);
		OLED_I2C_ReceiveACK();
   }
	OLED_I2C_Stop();
}
/**
  * @brief  设置OLED光标位置以方便写入
	* @param  Line:页地址, Range:0-7
	* @param  Column:列地址, Range:0-127
  * @retval 无
	**/
void OLED_SetCursor(uint8_t Page, uint8_t Column)
{
	OLED_WriteCommand(0xB0 | Page);					//设置Page位置
	OLED_WriteCommand(0x10 | ((Column & 0xF0) >> 4));	//设置Column位置高4位
	OLED_WriteCommand(0x00 | (Column & 0x0F));			//设置Column位置低4位
}

/**
	* @brief  OLED显示图片
	* @param  Line:显示起始行号,range:0 ~ 63
	* @param  Column:显示起始列号,range:0 ~ 127
	* @param  Width:图片宽度,range:0 ~ 128
	* @param  Height:图片高度,range:0 ~ 8
	* @param	const uint8_t (*Image)[8*Width]:图片数据的指针数组，注意数组的大小
  * @retval 无
  */
void OLED_ShowImage(uint8_t Line, uint8_t Column, uint8_t Width, uint8_t Height, const uint8_t (*Image)[Width])
{
	
	if (Line > OLED_MAX_LINES || Column  >  OLED_MAX_COLUMNS  ){	// 确保 Page 和 Column 不越界
		return;
	}
	OLED_ClearArea(Line,Column,Width,Height);
	for (uint8_t i = 0; i < Height; i++) {	
		
		for (uint8_t j = 0; j < Width ; j++) {				
			uint8_t tLine=(i * 8 + Line) / 8;
			OLED_SHOWBUFFER[ tLine ][j + Column]|= Image[i][j] << (Line % 8 );
			OLED_SHOWBUFFER[ tLine + 1][j + Column]|= Image[i][j] >> (8 - Line % 8);				
		}			
    }
}


/**
	* @brief  在OLED上显示一个字符
	* @param  Line:显示起始行号
	* @param  Column:显示起始列号
	* @param  Char:需要显示的原始字符数据
	* @param  FontSize:字符大小,可选OLED6X8,OLED8X16,按需求配置
  * @retval 无
  */
void OLED_ShowChar(uint8_t Line,uint8_t Column,uint8_t Char,uint8_t FontSize)
{
	//按字符大小对OLED_ShowImage()函数中高度与宽度进行修改
	if(FontSize == OLED6X8)  OLED_ShowImage( Line * 8 ,Column * OLED6X8 , OLED6X8 ,1 ,Ascii_F6x8  + Char - 32);
	if(FontSize == OLED8X16) OLED_ShowImage( Line * 16,Column * OLED8X16, OLED8X16 ,2 ,Ascii_F8x16 + Char - 32);
}
/**
	* @brief  在OLED上显示十六进制数
	* @param  Line:显示起始行号
	* @param  Column:显示起始列号
	* @param  HexNum:需要显示的十六进制数
	* @param  FontSize:字符大小,可选OLED6X8,OLED8X16,按需求配置
  * @retval 无
  */
void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint8_t HexNum,uint8_t FontSize)
{
    char HighNibble = (HexNum & 0xF0) >> 4;
    char LowNibble = HexNum & 0x0F;

    if (HighNibble < 10)
			OLED_ShowChar(Line,Column,HighNibble + 0x30,FontSize);	// 0-9
    else
			OLED_ShowChar(Line,Column,HighNibble - 10 + 'A',FontSize);// A-F

    if (LowNibble < 10)
			OLED_ShowChar(Line,Column + 1,LowNibble + 0x30,FontSize);	// 0-9
    else
			OLED_ShowChar(Line,Column + 1,LowNibble - 10 + 'A',FontSize);// A-F

}
/**
	* @brief  在OLED上显示二进制数
	* @param  Line:显示起始行号
	* @param  Column:显示起始列号
	* @param  BinNum:需要显示的二进制数
	* @param  FontSize:字符大小,可选OLED6X8,OLED8X16,按需求配置
  * @retval 无
  */
void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint8_t BinNum,uint8_t FontSize){
    char binaryDigits[8]; 	// 定义一个数组来存储二进制位
    // 将 BinNum 转换成二进制字符串
    for (int8_t i = 7; i >= 0; i--) {
        binaryDigits[7 - i] = ( (BinNum >> i) & 0x01 ) ? '1' : '0';
    }
    // 从左到右逐个显示二进制位
    for (uint8_t i = 0; i < 8; i++) {
        OLED_ShowChar(Line, Column + i, binaryDigits[i] ,FontSize);
    }
}

uint32_t Pow(uint32_t X, uint32_t Y)
{
	uint32_t Result = 1;	
	while (Y --)			
	{
		Result *= X;		
	}
	return Result;
}
/**
	* @brief  OLED显示无符号数字
	* @param  Line:显示起始行号,range:随FontSize改变
	* @param  Column:显示起始列号,range:随FontSize改变
	* @param  Num:显示数字
	* @param  Length:显示长度
	* @param  FontSize:字模大小,可选参数: OLED6X8,OLED8X16
  * @retval 无
  */
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Num, uint8_t Length,uint8_t FontSize){
	for(uint8_t i = 0;i < Length; i++){
		OLED_ShowChar(Line,Column + i,Num / Pow(10,(Length - i -1) ) % 10 + '0',FontSize);
	}
}
/**
	* @brief  OLED显示有符号数字
	* @param  Line:显示起始行号,range:随FontSize改变
	* @param  Column:显示起始列号,range:随FontSize改变
	* @param  Num:显示数字
	* @param  Length:显示长度
	* @param  FontSize:字模大小,可选参数: OLED6X8,OLED8X16
  * @retval 无
  */
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Num, uint8_t Length,uint8_t FontSize)
{ 
	uint8_t Offset=0;
	
	if(Num < 0){
		Num=-Num;
		OLED_ShowChar(Line,Column ,'-',FontSize);
		Offset=1;
	}
	OLED_ShowNum(Line,Column + Offset,Num,Length,FontSize);
	
}

void OLED_ShowFloatNum(uint8_t Line, uint8_t Column,double Number, uint8_t IntLength, uint8_t FraLength, uint8_t FontSize)
{
	uint16_t PowNum, IntNum,FraNum;
	
	if (Number < 0)
	{		
		OLED_ShowChar(Line,Column++, '-', FontSize);	//显示-号
		Number = -Number;					//Number取负
	}
		/*提取整数部分和小数部分*/
	IntNum = (uint32_t)Number;			//直接赋值给整型变量，提取整数
	PowNum = Pow(10, FraLength);		//根据指定小数的位数，确定乘数
	double fractional = Number - IntNum;
	FraNum = (uint32_t)(fractional * PowNum + 0.5);  // 手动四舍五入
	
	if(FraNum >= PowNum) {
        IntNum++;
        FraNum -= PowNum;
    }
/*显示整数部分*/
	OLED_ShowNum(Line ,Column++, IntNum, IntLength, FontSize);
	
	/*显示小数点*/
	OLED_ShowChar(Line, Column++, '.', FontSize);
	
	/*显示小数部分*/
	OLED_ShowNum(Line, Column++, FraNum, FraLength, FontSize);
}

/**
	* @brief  OLED显示字符串
	* @param  Line:显示起始行号,range:随FontSize改变
	* @param  Column:显示起始列号,range:随FontSize改变
	* @param  String:字符串指针
	* @param  FontSize:字模大小,可选参数: OLED6X8,OLED8X16
  * @retval 无
  */
void OLED_ShowString(uint8_t Line ,uint8_t Column,char* String,uint8_t FontSize)
{
	for(uint8_t i = 0; ; i++){
		if(String[i] != '\0'){
			OLED_ShowChar(Line,Column + i,String[i],FontSize);
		}
		else{ break; }
  }
}
/**
	* @brief  OLED显示中文字符串
	* @param  Line:显示起始行号 ,Range:0 ~ 3
	* @param  Column:显示起始列号 ,Range:0 ~ 7
  * @retval 无
  */
void OLED_ShowChinese(uint8_t Line, uint8_t Column,const char* Chinese)
{
	uint8_t pChinese=0,pIndex=0,pChn16=0;
	char SingleChinese[ MAX_CHN_CHARLEN + 1 ] = {0};
	
	while(Chinese [pChinese] != '\0'){
		
		SingleChinese[pIndex++]=Chinese[pChinese++];
		
		if(pIndex==MAX_CHN_CHARLEN){
			
			while( strcmp ( "" , Chinese16x16[pChn16].Chinese ) !=0 ){
				if( strcmp ( SingleChinese , Chinese16x16[pChn16].Chinese ) == 0 )	break;
				pChn16 ++;
			}
			OLED_ShowImage(Line * 16,Column * 16,16,2,Chinese16x16[pChn16].Ch_Data);
			Column += 1;
			pIndex = 0;
			pChn16 = 0;
		}
	}
}

void OLED_Clear(void)
{  
	uint8_t i, j;
	for (j = 0; j < 8; j++){
		OLED_SetCursor(j, 0);
		for(i = 0; i < 128; i++){
			OLED_SHOWBUFFER[i][j]=0x00;
		}
	}
}
void OLED_ClearArea(uint8_t Line, uint8_t Column, uint8_t Width, uint8_t Height)
{  
	if (Line > OLED_MAX_LINES || Column >  OLED_MAX_COLUMNS  ){	// 确保 Page 和 Column 不越界
		return;
	}
	for (uint8_t i = 0; i < Height; i++) {			
		for (uint8_t j = 0; j < Width ; j++) {				
			uint8_t tLine=(i * 8 + Line) / 8;
			OLED_SHOWBUFFER[ tLine ][j + Column]= 0x00;
			OLED_SHOWBUFFER[ tLine + 1][j + Column]= 0x00;				
		}			
    }
}
void OLED_Update(void){
	for (uint8_t i = 0; i < 8; i++) {
		OLED_SetCursor(i,0);	
		OLED_WriteData( OLED_SHOWBUFFER[i] ,128);		// 写入图像数据
	}
}
void OLED_Init(void)
{	
	OLED_I2C_Init();
	
	OLED_WriteCommand(0xAE);	//关闭显示
	
	OLED_WriteCommand(0xD5);	//设置显示时钟分频比/振荡器频率
	OLED_WriteCommand(0x80);
	
	OLED_WriteCommand(0xA8);	//设置多路复用率
	OLED_WriteCommand(0x3F);
	
	OLED_WriteCommand(0xD3);	//设置显示偏移
	OLED_WriteCommand(0x00);
	
	OLED_WriteCommand(0x40);	//设置显示开始行
	
	OLED_WriteCommand(0xA1);	//设置左右方向，0xA1正常 0xA0左右反置
	
	OLED_WriteCommand(0xC8);	//设置上下方向，0xC8正常 0xC0上下反置

	OLED_WriteCommand(0xDA);	//设置COM引脚硬件配置
	OLED_WriteCommand(0x12);
	
	OLED_WriteCommand(0x81);	//设置对比度控制
	OLED_WriteCommand(0xCF);

	OLED_WriteCommand(0xD9);	//设置预充电周期
	OLED_WriteCommand(0xF1);

	OLED_WriteCommand(0xDB);	//设置VCOMH取消选择级别
	OLED_WriteCommand(0x30);

	OLED_WriteCommand(0xA4);	//设置整个显示打开/关闭

	OLED_WriteCommand(0xA6);	//设置正常/倒转显示

	OLED_WriteCommand(0x8D);	//设置充电泵
	OLED_WriteCommand(0x14);

	OLED_WriteCommand(0xAF);	//开启显示
	OLED_Clear();				//OLED清屏
}


