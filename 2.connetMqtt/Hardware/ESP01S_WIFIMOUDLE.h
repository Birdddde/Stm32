#ifndef __ESP01S_H
#define __ESP01S_H

/*WIFI_Config**********************************************************************************/
#define CWMODE "1"	//WIFI模式，可选：0(non-WIFI)、1(Station)、2(SoftAP)、3(SoftAP+Station)
/*Alicloud_config******************************************************************************/
#define SNTPADD "\"ntp1.aliyun.com\"" //设置SNTP服务器地址
#define SNTPSTRING "1,8," SNTPADD //设置SNTP服务器地址

/*Functions************************************************************************************/
void Esp01s_SendCommand(char* Command,uint8_t IsSet,char* Content);
uint8_t Esp01s_TestAT(void);
void Esp01s_SetCWMODE(void);
void Esp01s_SetSNTPServer(void);
void Esp01s_ConnectWifi(void);
void Esp01s_ConnectMQTT(void);
void MQTT_Subscribe(void);
void MQTT_UploadNon(uint8_t Non);
#endif
