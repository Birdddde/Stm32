#ifndef __ESP01S_H
#define __ESP01S_H

/*WIFI_Config**********************************************************************************/


#define CWMODE "1"	//WIFI模式，可选：0(non-WIFI)、1(Station)、2(SoftAP)、3(SoftAP+Station)
/*Alicloud_config******************************************************************************/
#define SNTPADD "\"ntp1.aliyun.com\"" //设置SNTP服务器地址
#define SNTPSTRING "1,8," SNTPADD //设置SNTP服务器地址

/*Functions************************************************************************************/
uint8_t Esp01s_ConnectAli(uint8_t* error);
uint8_t MQTT_Subscribe(void);
uint8_t MQTT_UploadNon(uint8_t Non);
uint8_t MQTT_UploadState(uint8_t State);

#endif
