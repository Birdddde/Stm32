#include "USART.h"
#include "delay.h"
#include "ESP01S_WIFIMOUDLE.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_BUFF_LEN 200 

char gEsp01s_buff[MAX_BUFF_LEN]={0};	//发送字符串缓冲区，用于拼接字符串
/*需提供的参数************************************************************************************/
char* gSSID = "\"iphone18\"";					//需要连接的Wifi名称
char* gSSID_PASS = "\"mxbctmm666\"";		//Wifi密码
char* gMQTT_CLIENTID = "\"FESA234FBDS24|securemode=3\\,signmethod=hmacsha1\\,timestamp=789|\"" ;
char* gMQTT_USERNAME = "\"device_esp8266&k1xclVsAeX1\"" ;
char* gMQTT_PASSWD = "\"18f74facb3c7fdc160260feb5ba7985582c50ed8\"" ;
char* gMQTT_HOSTURL = "\"iot-06z00ba39al5xxu.mqtt.iothub.aliyuncs.com\"";
char* gMQTT_PORT =  "1883";
/*订阅与发布的路径*********************************************************************************/
char* gMQTT_SUBPATH =  "\"sys/k1xclVsAeX1/device_esp8266/thing/service/property/set\"";
char* gMQTT_PUBPATH =  "\"/sys/k1xclVsAeX1/device_esp8266/thing/event/property/post\"";
/*Functions****************************************************************************************/

/**
 * 发送 ESP01S 模块的 AT 命令
 *
 * @param Command   AT 命令名称
 * @param IsSet     是否设置参数 (0: 不设置, 1: 设置)
 * @param Content   设置的参数内容 (仅当 IsSet 为 1 时有效)
 */
void Esp01s_SendCommand(char* Command,uint8_t IsSet,char* Content)
{
	if(!IsSet){							// 构建不带参数的 AT 命令
		Serial_SendString("AT+");
		Serial_SendString(Command);
		Serial_SendString("\r\n");
	}else{								// 构建带参数的 AT 命令				
		Serial_SendString("AT+");
		Serial_SendString(Command);
		Serial_SendString("=");
		Serial_SendString(Content);
		Serial_SendString("\r\n");
	}
	memset(gEsp01s_buff,0,sizeof(gEsp01s_buff));	    // 清空缓冲区
}
/**
 * 测试 ESP01S 模块的 AT 命令
 * @param 无
 * @retval 测试成功返回1，否则为0
 */
uint8_t Esp01s_TestAT(void)
{
	Serial_SendString("AT\r\n");
	Delay_ms(500);
	if(Serial_GetRxFlag() == 1)
	{
		if( strstr(Receive,"OK") ){
			return 1;
		}
	}
	return 0;
}

void Esp01s_SetCWMODE(void)
{
	Esp01s_SendCommand("CWMODE",1,CWMODE);
}

void Esp01s_SetSNTPServer(void)
{
	Esp01s_SendCommand("CIPSNTPCFG",1,SNTPSTRING);
}

void Esp01s_ConnectWifi(void) {
	snprintf(gEsp01s_buff, MAX_BUFF_LEN, "%s,%s", gSSID, gSSID_PASS);// 使用 snprintf 进行安全的字符串拼接
	Esp01s_SendCommand("CWJAP", 1, gEsp01s_buff);
}
void Esp01s_ConnectMQTT(void)
{
	snprintf(gEsp01s_buff, MAX_BUFF_LEN, "%s,%s,%s,%s" ,"0,1,\"NULL\"", gMQTT_USERNAME,gMQTT_PASSWD,"0,0,\"\"");
	Esp01s_SendCommand("MQTTUSERCFG",1,gEsp01s_buff);
	Delay_ms(1000);
	snprintf(gEsp01s_buff, MAX_BUFF_LEN, "%s,%s" ,"0", gMQTT_CLIENTID);// 使用 snprintf 进行安全的字符串拼接
	Esp01s_SendCommand("MQTTCLIENTID",1,gEsp01s_buff);
	Delay_ms(1000);
	snprintf(gEsp01s_buff,MAX_BUFF_LEN, "%s,%s,%s,%s" ,"0",gMQTT_HOSTURL,gMQTT_PORT,"1");// 使用 snprintf 进行安全的字符串拼接
	Esp01s_SendCommand("MQTTCONN",1,gEsp01s_buff);
	Delay_ms(2000);
}

void MQTT_Subscribe(void)
{
	snprintf(gEsp01s_buff,MAX_BUFF_LEN, "%s,%s,%s" ,"0",gMQTT_SUBPATH,"1");// 使用 snprintf 进行安全的字符串拼接
	Esp01s_SendCommand("MQTTSUB",1,gEsp01s_buff);
}

void MQTT_UploadNon(uint8_t Non)
{
	snprintf(gEsp01s_buff,MAX_BUFF_LEN, "%s,%s,%s%u%s" ,"0",gMQTT_PUBPATH,"\"{\\\"params\\\":{\\\"non\\\":",Non,"}}\",1,0");// 字符串拼接为JSON格式报文
	Esp01s_SendCommand("MQTTPUB",1,gEsp01s_buff);
}

