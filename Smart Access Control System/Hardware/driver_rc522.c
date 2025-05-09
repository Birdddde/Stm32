#include "stm32f10x.h"                  // Device header
#include "delay.h"                  // Device header
#include "rc522.h"                  // Device header
#include <stdio.h>

/* 接线说明 
 RC522     	STM32F103C8T6
 SDA	--->	PA4
 SCK	--->	PA5
 MOSI	--->	PA7
 MISO	--->	PA6
 RST	--->	PB12
*/

/* IO口操作函数 */
#define   RC522_CS_Enable()         GPIO_ResetBits ( GPIOA, GPIO_Pin_4 )
#define   RC522_CS_Disable()        GPIO_SetBits ( GPIOA, GPIO_Pin_4 )

#define   RC522_Reset_Enable()      GPIO_ResetBits( GPIOB, GPIO_Pin_12 )
#define   RC522_Reset_Disable()     GPIO_SetBits( GPIOB, GPIO_Pin_12 )

#define   RC522_SCK_0()             GPIO_ResetBits( GPIOA, GPIO_Pin_5 )
#define   RC522_SCK_1()             GPIO_SetBits( GPIOA, GPIO_Pin_5 )

#define   RC522_MOSI_0()            GPIO_ResetBits( GPIOA, GPIO_Pin_7 )
#define   RC522_MOSI_1()            GPIO_SetBits( GPIOA, GPIO_Pin_7 )

#define   RC522_MISO_GET()          GPIO_ReadInputDataBit( GPIOA, GPIO_Pin_6 )

#define   RC522_Delay()          Delay_us(200)

/**
  * @brief  初始化RC522的GPIO
  * @param  无
  * @retval 无
  */
void RC522_GPIO_Init( void )
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB, ENABLE );
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_Init( GPIOA, &GPIO_InitStructure );
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_Init( GPIOA, &GPIO_InitStructure );
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_Init( GPIOA, &GPIO_InitStructure );
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_Init( GPIOB, &GPIO_InitStructure );
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_Init( GPIOA, &GPIO_InitStructure );
}

/**
  * @brief  软件模拟SPI发送一个字节数据，高位先行
  * @param  byte: 要发送的字节
  * @retval 无
  */
void RC522_SPI_SendByte( uint8_t byte )
{
	uint8_t n;
	for( n=0;n<8;n++ )
	{
		if( byte&0x80 )
			RC522_MOSI_1();
		else
			RC522_MOSI_0();
		
		RC522_Delay();
		
		RC522_SCK_0();
		RC522_Delay();
		
		RC522_SCK_1();
		RC522_Delay();
		
		byte<<=1;
	}
}

/**
  * @brief  软件模拟SPI读取一个字节数据，先读高位
  * @param  无
  * @retval 读取的字节
  */
uint8_t RC522_SPI_ReadByte( void )
{
	uint8_t n,data;
	for( n=0;n<8;n++ )
	{
		data<<=1;
		RC522_SCK_0();
		RC522_Delay();
		
		if( RC522_MISO_GET()==1 )
			data|=0x01;
		
		RC522_Delay();
		RC522_SCK_1();
		RC522_Delay();
	
	}
	return data;
}

/**
  * @brief  读取RC522的寄存器
  * @param  Address: 寄存器地址
  * @retval 读取的字节
  */
uint8_t RC522_Read_Register( uint8_t Address )
{
	uint8_t data,Addr;
	
	Addr = ( (Address<<1)&0x7E )|0x80;
	
	RC522_CS_Enable();
	RC522_SPI_SendByte( Addr );
	data = RC522_SPI_ReadByte();//读取寄存器中的值
	RC522_CS_Disable();
	
	return data;
}

/**
  * @brief  写入RC522的寄存器
  * @param  Address: 寄存器地址
  * @param  data: 要写入的数据
  * @retval 无
  */
void RC522_Write_Register( uint8_t Address, uint8_t data )
{
	uint8_t Addr;
	
	Addr = ( Address<<1 )& 0x7E;
	
	RC522_CS_Enable();
	RC522_SPI_SendByte( Addr );
	RC522_SPI_SendByte( data );
	RC522_CS_Disable();
}

/**
  * @brief  设置RC522的寄存器位
  * @param  Address: 寄存器地址
  * @param  mask: 要设置的位
  * @retval 无
  */
void RC522_SetBit_Register( uint8_t Address, uint8_t mask ){
	  uint8_t temp;       
	  temp = RC522_Read_Register( Address );/* 获取寄存器当前值 */        
	  RC522_Write_Register( Address, temp|mask );/* 对指定位进行置位操作后，再将值写入寄存器 */
}

/**
  * @brief  清除RC522的寄存器位
  * @param  Address: 寄存器地址
  * @param  mask: 要清除的位
  * @retval 无
  */
void RC522_ClearBit_Register( uint8_t Address, uint8_t mask ){
  uint8_t temp;
  temp = RC522_Read_Register( Address );/* 获取寄存器当前值 */
  RC522_Write_Register( Address, temp&(~mask) );  /* 对指定位进行清位操作后，再将值写入寄存器 */
}

/**
  * @brief  重置RC522
  * @param  无
  * @retval 无
  */
void RC522_Reset( void )
{
	RC522_Reset_Disable();
	Delay_us ( 1 );
	RC522_Reset_Enable();
	Delay_us ( 1 );
	RC522_Reset_Disable();
	Delay_us ( 1 );
	RC522_Write_Register( CommandReg, 0x0F );
	while( RC522_Read_Register( CommandReg )&0x10 )
		;

	/* 缓冲一下 */
	Delay_us ( 1 );
	RC522_Write_Register( ModeReg, 0x3D );       //定义发送和接收常用模式
	RC522_Write_Register( TReloadRegL, 30 );     //16位定时器低位
	RC522_Write_Register( TReloadRegH, 0 );      //16位定时器高位
	RC522_Write_Register( TModeReg, 0x8D );      //内部定时器的设置
	RC522_Write_Register( TPrescalerReg, 0x3E ); //设置定时器分频系数
	RC522_Write_Register( TxAutoReg, 0x40 );     //调制发送信号为100%ASK
}

/**
  * @brief  开启天线
  * @param  无
  * @retval 无
  */
void RC522_Antenna_On( void )
{
  uint8_t k;
  k = RC522_Read_Register( TxControlReg );
  
  if( !( k&0x03 ) )/* 判断天线是否开启 */
	 RC522_SetBit_Register( TxControlReg, 0x03 );
}

/**
  * @brief  关闭天线
  * @param  无
  * @retval 无
  */
void RC522_Antenna_Off( void ){       
  RC522_ClearBit_Register( TxControlReg, 0x03 );/* 直接对相应位清零 */
}

/**
  * @brief  配置RC522的类型
  * @param  Type: 类型
  * @retval 无
  */
void RC522_Config_Type( char Type )
{
        if( Type=='A' )
        {
                RC522_ClearBit_Register( Status2Reg, 0x08 );
                RC522_Write_Register( ModeReg, 0x3D );
                RC522_Write_Register( RxSelReg, 0x86 );
                RC522_Write_Register( RFCfgReg, 0x7F );
                RC522_Write_Register( TReloadRegL, 30 );
                RC522_Write_Register( TReloadRegH, 0 );
                RC522_Write_Register( TModeReg, 0x8D );
                RC522_Write_Register( TPrescalerReg, 0x3E );
                Delay_us(2);
                /* 开天线 */
                RC522_Antenna_On();
        }
}
/////////////////////////STM32控制RC522与M1卡的通信///////////////////////////////////////
/*
    通过RC522和M1卡通讯（数据的双向传输）
    寻卡
    防冲突
    用RC522计算CRC16（循环冗余校验）
    选定卡片
    校验卡片密码
    在M1卡的指定块地址写入指定数据
    读取M1卡的指定块地址的数据
    让卡片进入休眠模式
*/

/**
  * @brief  ：通过RC522和ISO14443卡通讯
* @param  ：ucCommand：RC522命令字
 *          pInData：通过RC522发送到卡片的数据
 *          ucInLenByte：发送数据的字节长度
 *          pOutData：接收到的卡片返回数据
 *          pOutLenBit：返回数据的位长度
  * @retval ：状态值MI_OK，成功
*/
char PcdComMF522 ( uint8_t ucCommand, uint8_t * pInData, uint8_t ucInLenByte, uint8_t * pOutData, uint32_t * pOutLenBit )
{
    char cStatus = MI_ERR;
    uint8_t ucIrqEn   = 0x00;
    uint8_t ucWaitFor = 0x00;
    uint8_t ucLastBits;
    uint8_t ucN;
    uint32_t ul;


    switch ( ucCommand )
    {
       case PCD_AUTHENT:                //Mifare认证
          ucIrqEn   = 0x12;                //允许错误中断请求ErrIEn  允许空闲中断IdleIEn
          ucWaitFor = 0x10;                //认证寻卡等待时候 查询空闲中断标志位
          break;

       case PCD_TRANSCEIVE:                //接收发送 发送接收
          ucIrqEn   = 0x77;                //允许TxIEn RxIEn IdleIEn LoAlertIEn ErrIEn TimerIEn
          ucWaitFor = 0x30;                //寻卡等待时候 查询接收中断标志位与 空闲中断标志位
          break;

       default:
         break;

    }

    RC522_Write_Register ( ComIEnReg, ucIrqEn | 0x80 );                //IRqInv置位管脚IRQ与Status1Reg的IRq位的值相反
    RC522_ClearBit_Register ( ComIrqReg, 0x80 );                        //Set1该位清零时，CommIRqReg的屏蔽位清零
    RC522_Write_Register ( CommandReg, PCD_IDLE );                //写空闲命令
    RC522_SetBit_Register ( FIFOLevelReg, 0x80 );                        //置位FlushBuffer清除内部FIFO的读和写指针以及ErrReg的BufferOvfl标志位被清除

    for ( ul = 0; ul < ucInLenByte; ul ++ )
                  RC522_Write_Register ( FIFODataReg, pInData [ ul ] );                    //写数据进FIFOdata

    RC522_Write_Register ( CommandReg, ucCommand );                                        //写命令


    if ( ucCommand == PCD_TRANSCEIVE )
                        RC522_SetBit_Register(BitFramingReg,0x80);                                  //StartSend置位启动数据发送 该位与收发命令使用时才有效

    ul = 1000;//根据时钟频率调整，操作M1卡最大等待时间25ms

    do                                                                                                                 //认证 与寻卡等待时间
    {
         ucN = RC522_Read_Register ( ComIrqReg );                                                        //查询事件中断
         ul --;
    } while ( ( ul != 0 ) && ( ! ( ucN & 0x01 ) ) && ( ! ( ucN & ucWaitFor ) ) );                //退出条件i=0,定时器中断，与写空闲命令

    RC522_ClearBit_Register ( BitFramingReg, 0x80 );                                        //清理允许StartSend位

    if ( ul != 0 )
    {
        if ( ! ( RC522_Read_Register ( ErrorReg ) & 0x1B ) )                        //读错误标志寄存器BufferOfI CollErr ParityErr ProtocolErr
        {
        cStatus = MI_OK;

        if ( ucN & ucIrqEn & 0x01 )                                        //是否发生定时器中断
          cStatus = MI_NOTAGERR;

        if ( ucCommand == PCD_TRANSCEIVE )
        {
                ucN = RC522_Read_Register ( FIFOLevelReg );                        //读FIFO中保存的字节数

                ucLastBits = RC522_Read_Register ( ControlReg ) & 0x07;        //最后接收到得字节的有效位数

                if ( ucLastBits )
                        * pOutLenBit = ( ucN - 1 ) * 8 + ucLastBits;           //N个字节数减去1（最后一个字节）+最后一位的位数 读取到的数据总位数
                else
                        * pOutLenBit = ucN * 8;                                           //最后接收到的字节整个字节有效

                if ( ucN == 0 )
        ucN = 1;

                if ( ucN > MAXRLEN )
                        ucN = MAXRLEN;

                for ( ul = 0; ul < ucN; ul ++ )
                  pOutData [ ul ] = RC522_Read_Register ( FIFODataReg );
                }
        }
        else
        cStatus = MI_ERR;
    }

   RC522_SetBit_Register ( ControlReg, 0x80 );           // stop timer now
   RC522_Write_Register ( CommandReg, PCD_IDLE );

   return cStatus;
}



/**
  * @brief  ：寻卡
* @param  ucReq_code，寻卡方式
*                      = 0x52：寻感应区内所有符合14443A标准的卡
 *                     = 0x26：寻未进入休眠状态的卡
 *         pTagType，卡片类型代码
 *                   = 0x4400：Mifare_UltraLight
 *                   = 0x0400：Mifare_One(S50)
 *                   = 0x0200：Mifare_One(S70)
 *                   = 0x0800：Mifare_Pro(X))
 *                   = 0x4403：Mifare_DESFire
  * @retval ：状态值MI_OK，成功
*/
char PcdRequest ( uint8_t ucReq_code, uint8_t * pTagType )
{
   char cStatus;
   uint8_t ucComMF522Buf [ MAXRLEN ];
   uint32_t ulLen;

   RC522_ClearBit_Register ( Status2Reg, 0x08 );        //清理指示MIFARECyptol单元接通以及所有卡的数据通信被加密的情况
   RC522_Write_Register ( BitFramingReg, 0x07 );        //        发送的最后一个字节的 七位
   RC522_SetBit_Register ( TxControlReg, 0x03 );        //TX1,TX2管脚的输出信号传递经发送调制的13.46的能量载波信号

   ucComMF522Buf [ 0 ] = ucReq_code;                //存入寻卡方式
        /* PCD_TRANSCEIVE：发送并接收数据的命令，RC522向卡片发送寻卡命令，卡片返回卡的型号代码到ucComMF522Buf中 */
   cStatus = PcdComMF522 ( PCD_TRANSCEIVE,        ucComMF522Buf, 1, ucComMF522Buf, & ulLen );        //寻卡

   if ( ( cStatus == MI_OK ) && ( ulLen == 0x10 ) )        //寻卡成功返回卡类型
   {
                 /* 接收卡片的型号代码 */
       * pTagType = ucComMF522Buf [ 0 ];
       * ( pTagType + 1 ) = ucComMF522Buf [ 1 ];
   }
   else
     cStatus = MI_ERR;
     return cStatus;
}

/**
  * @brief  ：防冲突
        * @param  ：Snr：卡片序列，4字节，会返回选中卡片的序列
  * @retval ：状态值MI_OK，成功
*/
char PcdAnticoll ( uint8_t * pSnr )
{
    char cStatus;
    uint8_t uc, ucSnr_check = 0;
    uint8_t ucComMF522Buf [ MAXRLEN ];
          uint32_t ulLen;

    RC522_ClearBit_Register ( Status2Reg, 0x08 );                //清MFCryptol On位 只有成功执行MFAuthent命令后，该位才能置位
    RC522_Write_Register ( BitFramingReg, 0x00);                //清理寄存器 停止收发
    RC522_ClearBit_Register ( CollReg, 0x80 );                        //清ValuesAfterColl所有接收的位在冲突后被清除

    ucComMF522Buf [ 0 ] = 0x93;        //卡片防冲突命令
    ucComMF522Buf [ 1 ] = 0x20;

          /* 将卡片防冲突命令通过RC522传到卡片中，返回的是被选中卡片的序列 */
    cStatus = PcdComMF522 ( PCD_TRANSCEIVE, ucComMF522Buf, 2, ucComMF522Buf, & ulLen);//与卡片通信

    if ( cStatus == MI_OK)                //通信成功
    {
                        for ( uc = 0; uc < 4; uc ++ )
                        {
         * ( pSnr + uc )  = ucComMF522Buf [ uc ];                        //读出UID
         ucSnr_check ^= ucComMF522Buf [ uc ];
      }

      if ( ucSnr_check != ucComMF522Buf [ uc ] )
                                cStatus = MI_ERR;
    }
    RC522_SetBit_Register ( CollReg, 0x80 );
    return cStatus;
}



/**
 * @brief   :用RC522计算CRC16（循环冗余校验）
        * @param  ：pIndata：计算CRC16的数组
 *            ucLen：计算CRC16的数组字节长度
 *            pOutData：存放计算结果存放的首地址
  * @retval ：状态值MI_OK，成功
*/
void CalulateCRC ( uint8_t * pIndata, u8 ucLen, uint8_t * pOutData )
{
    uint8_t uc, ucN;


    RC522_ClearBit_Register(DivIrqReg,0x04);
    RC522_Write_Register(CommandReg,PCD_IDLE);
    RC522_SetBit_Register(FIFOLevelReg,0x80);

    for ( uc = 0; uc < ucLen; uc ++)
            RC522_Write_Register ( FIFODataReg, * ( pIndata + uc ) );

    RC522_Write_Register ( CommandReg, PCD_CALCCRC );

    uc = 0xFF;

    do
    {
        ucN = RC522_Read_Register ( DivIrqReg );
        uc --;
    } while ( ( uc != 0 ) && ! ( ucN & 0x04 ) );

    pOutData [ 0 ] = RC522_Read_Register ( CRCResultRegL );
    pOutData [ 1 ] = RC522_Read_Register ( CRCResultRegM );

}



/**
  * @brief   :选定卡片
  * @param  ：pSnr：卡片序列号，4字节
  * @retval ：状态值MI_OK，成功
*/
char PcdSelect ( uint8_t * pSnr )
{
    char ucN;
    uint8_t uc;
          uint8_t ucComMF522Buf [ MAXRLEN ];
    uint32_t  ulLen;
    /* PICC_ANTICOLL1：防冲突命令 */
    ucComMF522Buf [ 0 ] = PICC_ANTICOLL1;
    ucComMF522Buf [ 1 ] = 0x70;
    ucComMF522Buf [ 6 ] = 0;

    for ( uc = 0; uc < 4; uc ++ )
    {
            ucComMF522Buf [ uc + 2 ] = * ( pSnr + uc );
            ucComMF522Buf [ 6 ] ^= * ( pSnr + uc );
    }

    CalulateCRC ( ucComMF522Buf, 7, & ucComMF522Buf [ 7 ] );

    RC522_ClearBit_Register ( Status2Reg, 0x08 );

    ucN = PcdComMF522 ( PCD_TRANSCEIVE, ucComMF522Buf, 9, ucComMF522Buf, & ulLen );

    if ( ( ucN == MI_OK ) && ( ulLen == 0x18 ) )
      ucN = MI_OK;
    else
      ucN = MI_ERR;

    return ucN;

}



/**
  * @brief   :校验卡片密码
  * @param  ：ucAuth_mode：密码验证模式
  *                     = 0x60，验证A密钥
  *                     = 0x61，验证B密钥
  *           ucAddr：块地址
  *           pKey：密码
  *           pSnr：卡片序列号，4字节
  * @retval ：状态值MI_OK，成功
*/
char PcdAuthState ( uint8_t ucAuth_mode, uint8_t ucAddr, uint8_t * pKey, uint8_t * pSnr )
 {
    char cStatus;
          uint8_t uc, ucComMF522Buf [ MAXRLEN ];
    uint32_t ulLen;

    ucComMF522Buf [ 0 ] = ucAuth_mode;
    ucComMF522Buf [ 1 ] = ucAddr;
          /* 前俩字节存储验证模式和块地址，2~8字节存储密码（6个字节），8~14字节存储序列号 */
    for ( uc = 0; uc < 6; uc ++ )
            ucComMF522Buf [ uc + 2 ] = * ( pKey + uc );

    for ( uc = 0; uc < 6; uc ++ )
            ucComMF522Buf [ uc + 8 ] = * ( pSnr + uc );
    /* 进行冗余校验，14~16俩个字节存储校验结果 */
    cStatus = PcdComMF522 ( PCD_AUTHENT, ucComMF522Buf, 12, ucComMF522Buf, & ulLen );
          /* 判断验证是否成功 */
    if ( ( cStatus != MI_OK ) || ( ! ( RC522_Read_Register ( Status2Reg ) & 0x08 ) ) )
      cStatus = MI_ERR;

    return cStatus;

}


/**
  * @brief   :在M1卡的指定块地址写入指定数据
  * @param  ：ucAddr：块地址
  *           pData：写入的数据，16字节
  * @retval ：状态值MI_OK，成功
*/
char PcdWrite ( uint8_t ucAddr, uint8_t * pData )
{
    char cStatus;
          uint8_t uc, ucComMF522Buf [ MAXRLEN ];
    uint32_t ulLen;

    ucComMF522Buf [ 0 ] = PICC_WRITE;//写块命令
    ucComMF522Buf [ 1 ] = ucAddr;//写块地址

          /* 进行循环冗余校验，将结果存储在& ucComMF522Buf [ 2 ] */
    CalulateCRC ( ucComMF522Buf, 2, & ucComMF522Buf [ 2 ] );

        /* PCD_TRANSCEIVE:发送并接收数据命令，通过RC522向卡片发送写块命令 */
    cStatus = PcdComMF522 ( PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, & ulLen );

                /* 通过卡片返回的信息判断，RC522是否与卡片正常通信 */
    if ( ( cStatus != MI_OK ) || ( ulLen != 4 ) || ( ( ucComMF522Buf [ 0 ] & 0x0F ) != 0x0A ) )
      cStatus = MI_ERR;

    if ( cStatus == MI_OK )
    {
                        //memcpy(ucComMF522Buf, pData, 16);
                        /* 将要写入的16字节的数据，传入ucComMF522Buf数组中 */
      for ( uc = 0; uc < 16; uc ++ )
                          ucComMF522Buf [ uc ] = * ( pData + uc );
                        /* 冗余校验 */
      CalulateCRC ( ucComMF522Buf, 16, & ucComMF522Buf [ 16 ] );
      /* 通过RC522，将16字节数据包括2字节校验结果写入卡片中 */
      cStatus = PcdComMF522 ( PCD_TRANSCEIVE, ucComMF522Buf, 18, ucComMF522Buf, & ulLen );
                        /* 判断写地址是否成功 */
                        if ( ( cStatus != MI_OK ) || ( ulLen != 4 ) || ( ( ucComMF522Buf [ 0 ] & 0x0F ) != 0x0A ) )
        cStatus = MI_ERR;
    }
    return cStatus;
}


/**
  * @brief   :读取M1卡的指定块地址的数据
  * @param  ：ucAddr：块地址
  *           pData：读出的数据，16字节
  * @retval ：状态值MI_OK，成功
*/
char PcdRead ( uint8_t ucAddr, uint8_t * pData )
{
    char cStatus;
          uint8_t uc, ucComMF522Buf [ MAXRLEN ];
    uint32_t ulLen;

    ucComMF522Buf [ 0 ] = PICC_READ;
    ucComMF522Buf [ 1 ] = ucAddr;
          /* 冗余校验 */
    CalulateCRC ( ucComMF522Buf, 2, & ucComMF522Buf [ 2 ] );
    /* 通过RC522将命令传给卡片 */
    cStatus = PcdComMF522 ( PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, & ulLen );

          /* 如果传输正常，将读取到的数据传入pData中 */
    if ( ( cStatus == MI_OK ) && ( ulLen == 0x90 ) )
    {
                        for ( uc = 0; uc < 16; uc ++ )
        * ( pData + uc ) = ucComMF522Buf [ uc ];
    }
    else
      cStatus = MI_ERR;

    return cStatus;

}


/**
  * @brief   :让卡片进入休眠模式
  * @param  ：无
  * @retval ：状态值MI_OK，成功
*/
char PcdHalt( void )
{
        uint8_t ucComMF522Buf [ MAXRLEN ];
        uint32_t  ulLen;

  ucComMF522Buf [ 0 ] = PICC_HALT;
  ucComMF522Buf [ 1 ] = 0;

  CalulateCRC ( ucComMF522Buf, 2, & ucComMF522Buf [ 2 ] );
         PcdComMF522 ( PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, & ulLen );

  return MI_OK;

}

/**
  * @brief  初始化RC522
  * @param  无
  * @retval 无
  */
void RC522_Init( void )
{
	RC522_GPIO_Init();
	RC522_Reset();
}

	
