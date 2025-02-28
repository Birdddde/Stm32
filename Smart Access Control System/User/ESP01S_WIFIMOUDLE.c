#include "uart1.h"
#include "delay.h"
#include "ESP01S_WIFIMOUDLE.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "freertos.h"
#include "task.h"

#define MAX_BUFF_LEN 200 

char gEsp01s_tx_buff[MAX_BUFF_LEN]={0};	//发送字符串缓冲区，用于拼接字符串
/*需提供的参数************************************************************************************/
char* gSSID = "\"xiaoxxx\"";					//需要连接的Wifi名称
char* gSSID_PASS = "\"12345678\"";		//Wifi密码
char* gMQTT_CLIENTID = "\"FESA234FBDS24|securemode=3\\,signmethod=hmacsha1\\,timestamp=789|\"" ;
char* gMQTT_USERNAME = "\"AccessControl&k1xclk3TfbJ\"" ;
char* gMQTT_PASSWD = "\"476d1a989b55c3b66faadc069a77d966083ebc5e\"" ;
char* gMQTT_HOSTURL = "\"k1xclk3TfbJ.iot-as-mqtt.cn-shanghai.aliyuncs.com\"";
char* gMQTT_PORT =  "1883";
/*订阅与发布的路径*********************************************************************************/
char* gMQTT_SUBPATH =  "\"/sys/k1xclk3TfbJ/AccessControl/thing/service/property/set\"";
char* gMQTT_PUBPATH =  "\"/sys/k1xclk3TfbJ/AccessControl/thing/event/property/post\"";

Uart_Rx_t g_xUart1_rx;


/*Functions****************************************************************************************/


uint8_t Esp01s_SendCommand(char* Command,uint8_t IsSet,char* Content,char* expected,uint32_t delay_ms,Uart_Rx_t* uart1_rx)
{	
	if(!IsSet){							// 构建不带参数的 AT 命令
		Serial1_SendString("AT+");
		Serial1_SendString(Command);
		Serial1_SendString("\r\n");
	}else{								// 构建带参数的 AT 命令				
		Serial1_SendString("AT+");
		Serial1_SendString(Command);
		Serial1_SendString("=");
		Serial1_SendString(Content);
		Serial1_SendString("\r\n");
	}
	memset(gEsp01s_tx_buff,0,sizeof(gEsp01s_tx_buff));	    // 清空发送缓冲区
	
	Delay_ms(delay_ms);
	
	if( USART1_GetRxData(uart1_rx) ){
		
		g_xUart1_rx.rx_buffer[g_xUart1_rx.rx_data_length + 1] = '\0';
		char* sBuffer = (char*) pvPortMalloc(g_xUart1_rx.rx_data_length + 1);
		memcpy(sBuffer,g_xUart1_rx.rx_buffer,g_xUart1_rx.rx_data_length + 1);
		
		if(strstr(sBuffer,expected)){
			vPortFree(sBuffer);
			return 1;	//接收到期待值
		}
		vPortFree(sBuffer);	
		return 2;	//接收到数据与期待值不同
	}
	return 0;	//未接收到数据
}
/**
 * 测试 ESP01S 模块的 AT 命令
 * @param 无
 * @retval 测试成功返回1，否则为0
 */
uint8_t Esp01s_TestAT(void)
{
	Serial1_SendString("AT\r\n");
	Delay_ms(500);
	if( USART1_GetRxData(&g_xUart1_rx) ){
		g_xUart1_rx.rx_buffer[g_xUart1_rx.rx_data_length + 1] = '\0';
		
		char* sBuffer = (char*) pvPortMalloc(g_xUart1_rx.rx_data_length + 1);
		memcpy(sBuffer,g_xUart1_rx.rx_buffer,g_xUart1_rx.rx_data_length + 1);
		
		if(strstr(sBuffer,"OK")){
			vPortFree(sBuffer);
			return 1;	//接收到OK
		}
		vPortFree(sBuffer);	
		return 2;	//未接收到OK
	}
	return 0;
}

uint8_t Esp01s_SetCWMODE(void)
{
	return Esp01s_SendCommand("CWMODE",1,CWMODE,"OK",500,&g_xUart1_rx);
}

uint8_t Esp01s_SetSNTPServer(void)
{
	return Esp01s_SendCommand("CIPSNTPCFG",1,SNTPSTRING,"OK",500,&g_xUart1_rx);
}

uint8_t Esp01s_ConnectWifi(void) {
	snprintf(gEsp01s_tx_buff, MAX_BUFF_LEN, "%s,%s", gSSID, gSSID_PASS);// 使用 snprintf 进行安全的字符串拼接
	return Esp01s_SendCommand("CWJAP", 1, gEsp01s_tx_buff,"OK",3000,&g_xUart1_rx);
}

//** 阿里云相关**//

uint8_t Esp01s_ConnectMQTT(void)
{
	snprintf(gEsp01s_tx_buff, MAX_BUFF_LEN, "%s,%s,%s,%s" ,"0,1,\"NULL\"", gMQTT_USERNAME,gMQTT_PASSWD,"0,0,\"\"");
	Esp01s_SendCommand("MQTTUSERCFG",1,gEsp01s_tx_buff,"OK",1000,&g_xUart1_rx);
	
	snprintf(gEsp01s_tx_buff, MAX_BUFF_LEN, "%s,%s" ,"0", gMQTT_CLIENTID);// 使用 snprintf 进行安全的字符串拼接
	Esp01s_SendCommand("MQTTCLIENTID",1,gEsp01s_tx_buff,"OK",2000,&g_xUart1_rx);
	
	snprintf(gEsp01s_tx_buff,MAX_BUFF_LEN, "%s,%s,%s,%s" ,"0",gMQTT_HOSTURL,gMQTT_PORT,"1");// 使用 snprintf 进行安全的字符串拼接
	return Esp01s_SendCommand("MQTTCONN",1,gEsp01s_tx_buff,"OK",6000,&g_xUart1_rx);
}

uint8_t MQTT_Subscribe(void)
{
	snprintf(gEsp01s_tx_buff,MAX_BUFF_LEN, "%s,%s,%s" ,"0",gMQTT_SUBPATH,"1");// 使用 snprintf 进行安全的字符串拼接
	return Esp01s_SendCommand("MQTTSUB",1,gEsp01s_tx_buff,"OK",1000,&g_xUart1_rx);
}

uint8_t MQTT_UploadState(uint8_t State)
{
	snprintf(gEsp01s_tx_buff,MAX_BUFF_LEN, "%s,%s,%s%u%s" ,"0",gMQTT_PUBPATH,"\"{\\\"params\\\":{\\\"LockState\\\":",State,"}}\",1,0");// 字符串拼接为JSON格式报文
	return Esp01s_SendCommand("MQTTPUB",1,gEsp01s_tx_buff,"OK",1000,&g_xUart1_rx);
}

uint8_t Esp01s_ConnectAli(uint8_t* error){
	uint8_t error_code;
	
	error_code = Esp01s_TestAT();
	if( error_code != 1){
		*error = error_code;
		return 1;
	};

	error_code = Esp01s_SetCWMODE();
	if( error_code != 1){
		*error = error_code;
		return 2;
	};	

	error_code = Esp01s_SetSNTPServer();	
	if( error_code != 1){
		*error = error_code;		
		return 3;
	};		

	error_code = Esp01s_ConnectWifi();
	if( error_code != 1){
		*error = error_code;
		return 4;
	};			
	
	error_code = Esp01s_SendCommand("MQTTCONN?",0,NULL,"OK",1500,&g_xUart1_rx);
	if( error_code == 1){
		Esp01s_SendCommand("MQTTCLEAN",1,"0","OK",1500,&g_xUart1_rx);
	};
		
	error_code = Esp01s_ConnectMQTT();
	if( error_code != 1){
		*error = error_code;
		return 5;
	};			
	
	error_code = MQTT_Subscribe();
	if( error_code != 1){
		*error = error_code;
		return 6;
	};
	return 0;
}

