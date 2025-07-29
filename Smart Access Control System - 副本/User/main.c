#include "stm32f10x.h"
#include <stdio.h>

#include "freertos.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"  
#include "tim.h"
#include "stm32f10x_pwr.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_iwdg.h"

#include "oled.h"
#include "face.h"
#include "finger.h"
#include "rfid.h"
#include "servo.h"
#include "beep.h"
#include "light.h"
#include "esp01s_wifimoudle.h"
#include "body_infrared.h"
#include "key.h"
#include "typedef.h"
#include "menu.h"
#include "uart1.h"
#include "storage.h"
#include "delay.h"

#define MENU_MAIN_LENGTH 3

TaskHandle_t g_xRC522Handle = NULL;
TaskHandle_t g_xMenuHandle = NULL;
TaskHandle_t g_xAs608Handle = NULL;
TaskHandle_t g_xLightHandle = NULL;
TaskHandle_t g_xK210Handle = NULL;
TaskHandle_t g_xWifiHandle = NULL;

wifi_error_t g_xError = {0,NULL};	//阿里云连接错误标志位
uint8_t g_ucDoor_status = 0;			//门锁状态标志位
uint8_t g_ucWifi_iscon_flag = 1;		//Wifi连接标志位
uint8_t g_ucWifi_testcon_flag = 0;	//Wifi测试连接标志位

SemaphoreHandle_t g_xMutex_Wifi,g_xMutex_Key,g_xOledMutex;
uint8_t g_ucaAdmin_pass[4];

volatile MenuState_t menuState = MENU_INACTIVE;
extern QueueHandle_t xQueueUart2;
extern uint16_t g_usFingerId;
extern xTimerHandle g_xPassLockTimer;

uint8_t g_uckey = 0;	//按键

// 定义一个足够大的缓冲区来存储任务信息
#define TASK_INFO_BUFFER_SIZE 512
char taskInfoBuffer[TASK_INFO_BUFFER_SIZE];

/* 运行时统计定时器配置 */
void ConfigureTimerForRunTimeStats(void)
{
    /* 使用TIM2作为运行时统计的定时器 */
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    
    /* 使能TIM2时钟 */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    
    /* 配置TIM2 */
    TIM_TimeBaseStructure.TIM_Period = 0xFFFF;  // 修改为16位最大值
    TIM_TimeBaseStructure.TIM_Prescaler = (SystemCoreClock / 1000000) - 1;  // 1MHz
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
    
    /* 启动TIM2 */
    TIM_Cmd(TIM2, ENABLE);
}

uint32_t GetRunTimeCounterValue(void)
{
    return TIM_GetCounter(TIM2);
}

void printTaskInfo(void)
{
    // 打印任务运行时统计
    #if (configGENERATE_RUN_TIME_STATS == 1)
        printf("\n=====Task Runtime Statistics=====\n");
        printf("Task Name\tRuntime Count\tUsage\n");
        vTaskGetRunTimeStats(taskInfoBuffer);
        printf("%s\n", taskInfoBuffer);
    #endif
    
    // 打印任务剩余栈空间
    printf("\n=====Task Stack Remaining=====\n");
    printf("Task Name\tStack Remaining\n");
    vTaskList(taskInfoBuffer);
    printf("%s\n", taskInfoBuffer);
}

void AS608Task(void *p){
	as608_status_t as608_status;
	uint8_t status = 0;
	uint16_t finger_id;
	
	while(1)
	{
		if(xTaskNotifyWait(0x01, 0x01, NULL, pdMS_TO_TICKS(100)) == pdPASS) 
		{			
			vTaskSuspend(g_xMenuHandle);	
			OLED_Clear();
			OLED_ShowString(0,30,"Flushing Finger...",OLED_6X8);
			OLED_Update();
			
			status = Finger_Flush(&as608_status,&finger_id,NULL);		
			OLED_ClearArea(0,30,128,8);
			if( status == FLUSH_FINGER_SUCCESS ){
				OLED_ShowString(0,30,"found",OLED_6X8);
				OLED_UpdateArea(0,30,128,8);
				if(g_ucWifi_iscon_flag){
					MQTT_UploadUnlockWay(0,finger_id,"");
				}
			}				
			if( status == FLUSH_FINGER_SEARCH_FAILED ){				
				OLED_ShowString(0,30,"not found",OLED_6X8);
				OLED_UpdateArea(0,30,128,8);
				Beep_On(1000);		
			}
			
			OLED_UpdateArea(0,30,128,8);
			OLED_Clear();
			OLED_Update();
			if( status == FLUSH_FINGER_SUCCESS )
				xTaskNotify(g_xMenuHandle,0x02,eSetBits);
				
				
			vTaskResume(g_xMenuHandle);				
		}		
	}			
}

void MenuTask(void *p){

	uint32_t ulNotificationValue;
	uint8_t Flag_Access = 0;
	
	while(1){
		
		if(xTaskNotifyWait(0x03, 0x03, &ulNotificationValue, 0) == pdPASS) {
			if(ulNotificationValue & 0x01){
				vTaskSuspend(g_xRC522Handle);	//暂停任务
				vTaskSuspend(g_xAs608Handle);
				vTaskSuspend(g_xK210Handle);
				vTaskSuspend(g_xWifiHandle);
				
				Menu_Init();
				menuState = MENU_ACTIVE;
				Display_Refresh();
			}
			if(ulNotificationValue & 0x02){
				Flag_Access = 1;
				Beep_On(50);
				Beep_On(50);
			}
		}
		
		
		if(menuState == MENU_ACTIVE){
			g_uckey = KeyNum_Get();
			if(g_uckey != KEY_NONE)	Key_Handler(g_uckey);

		   vTaskDelay(pdMS_TO_TICKS(10));
		}else
		{
			OLED_ShowImage(48,16,32,32,Lock32x32);
			
			if(g_ucWifi_iscon_flag != 1)	
				OLED_ShowImage(0,0,12,12,Wifi_Disconnected12x12);
			else
				OLED_ShowImage(0,0,12,12,Wifi_Connected12x12);
			
			if(xTimerIsTimerActive(g_xPassLockTimer) == pdTRUE)	
				OLED_ShowImage(15,0,12,12,Menu_CantAccess12x12);
			else
				OLED_ShowImage(15,0,12,12,Menu_CanAccess12x12);
			
			OLED_Update();
			
			if(Flag_Access){
				Servo_Control_Angle(180);
				g_ucDoor_status = 1;
				vTaskDelay(pdMS_TO_TICKS(2000));
				Flag_Access=0;
			}else{
				Servo_Control_Angle(0);
				g_ucDoor_status = 0;
			}			
			
			
			g_uckey = KeyNum_Get();
			if(g_uckey){	
				Pass_handlle(g_ucaAdmin_pass,Action_COMAPRE);
			}
		}
		
		vTaskDelay(pdMS_TO_TICKS(100)); // 非激活状态时降低检测频
	}
}

void RC522Task(void *p){
	static uint8_t ucStatus;
	static uint8_t CardId[4]={0};
	  char str[10];

	while(1){

		ucStatus = RFID_Scan(CardId);	
		
		if(ucStatus){
			ucStatus = 0;
			
			if(g_ucWifi_iscon_flag){
				sprintf(str,"%u%u%u%u",CardId[0],CardId[1],CardId[2],CardId[3]);
				MQTT_UploadUnlockWay(0,0,str);
			}
			
			xTaskNotify(g_xMenuHandle,0x02,eSetBits);
		}
		vTaskDelay(pdMS_TO_TICKS(1000)); // 非激活状态时降低检测频
	}
}

void K210Task(void *p){
	
	static uint8_t ucStatus;
	
	while(1){	
		ucStatus = Face_Scan();
		if(ucStatus){
			ucStatus = 0;
			
			if(g_ucWifi_iscon_flag){
				MQTT_UploadUnlockWay(1,0,"");
			}
			xTaskNotify(g_xMenuHandle,0x02,eSetBits);
		}
			
		vTaskDelay(pdMS_TO_TICKS(2000)); // 非激活状态时降低检测频
	}
}

void LightTask(void *p){
	TickType_t Tick=pdMS_TO_TICKS(5000);
	
	while(1){

		if( Body_Infra_GetData() == 1 )
			Light_On();
		else
			Light_Off();
		
		vTaskDelay(Tick);
	}
	
}

void WifiTask(void *p){
	Uart_Rx_t xUart1_rx;
	if(g_ucWifi_iscon_flag)
		MQTT_UploadPass(g_ucaAdmin_pass);

	while(1){
		if(g_ucWifi_testcon_flag)
		{
			g_ucWifi_iscon_flag = Esp01s_IsConnectWifi();
			g_ucWifi_testcon_flag = 0;
		}
			
		if(USART1_GetRxData(&xUart1_rx))
		{
			xUart1_rx.rx_buffer[xUart1_rx.rx_data_length + 1] = '\0';
			if(strstr((const char*)xUart1_rx.rx_buffer,"@get_password"))
				MQTT_UploadPass(g_ucaAdmin_pass);
			if(strstr((const char*)xUart1_rx.rx_buffer,"@set_lockstate\":1"))
				xTaskNotify(g_xMenuHandle,0x02,eSetBits);
			if(strstr((const char*)xUart1_rx.rx_buffer,"@set_lockstate\":0")){						
				Servo_Control_Angle(0);
				g_ucDoor_status = 0;
			}
		}
		vTaskDelay( pdMS_TO_TICKS(500) );
	}
	
}

/* 监控任务函数 */
void MonitorTask(void *p)
{
    /* 等待系统稳定运行 */
    vTaskDelay(pdMS_TO_TICKS(30000));  // 等待30秒，让系统稳定运行
    
    while(1)
    {
        /* 打印任务信息 */
        printTaskInfo();
        
        /* 每5分钟打印一次 */
        vTaskDelay(pdMS_TO_TICKS(300000));
    }
}

/* 看门狗初始化函数 */
void IWDG_Init(void)
{
    /* 使能LSI时钟 */
    RCC_LSICmd(ENABLE);
    while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET);
    
    /* 配置看门狗 */
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
    IWDG_SetPrescaler(IWDG_Prescaler_64);  // 64分频，增加超时时间
    IWDG_SetReload(0xFFF);  // 重载值，约3.2秒超时
    IWDG_ReloadCounter();
    IWDG_Enable();
}

/* 空闲任务钩子函数 */
void vApplicationIdleHook(void)
{
    static TickType_t xLastFeedTime = 0;
    TickType_t xCurrentTime = xTaskGetTickCount();
    
    /* 每1秒喂一次狗 */
    if((xCurrentTime - xLastFeedTime) > pdMS_TO_TICKS(1000))
    {
        IWDG_ReloadCounter();
        xLastFeedTime = xCurrentTime;
    }
}

int main(void)
{

	OLED_Init();
	RFID_Init();
   Key_Init();  
	Finger_Init();
	Face_Init();
	Storage_Init();
	Beep_Init();
	Servo_Init();
	Light_Init();
	Body_Infra_Init();
	ESP01S_Init();
	
	Delay_ms(2000);	//等待硬件初始化完成 
	
	g_xMutex_Key = xSemaphoreCreateMutex();
	g_xOledMutex = xSemaphoreCreateMutex();
	g_xMutex_Wifi= xSemaphoreCreateMutex();	//创建互斥信号量	

#ifdef WIFI	
	OLED_ShowChinese(40,32-8,"连接中");
	OLED_Update();

	Esp01s_ConnectAli(&g_xError);
	OLED_Clear();	//清屏
	OLED_Update();
#endif

	Timer_Create();	//创建软件定时器
	
	if (g_xError.error_code)
	{		
		OLED_ShowString(0,0,"Connect Aliyun failed",OLED_6X8);
		OLED_ShowString(0,8,"error code:",OLED_6X8);
		OLED_ShowString(0,16,"error src:",OLED_6X8);
		OLED_ShowNum(13*6,8,g_xError.error_code,2,OLED_6X8);
		OLED_ShowNum(13*6,16,g_xError.error_src,2,OLED_6X8);
		OLED_ShowString(0,24,"press any key to exit",OLED_6X8);
		OLED_Update();
		
		Delay_ms(5000);
		OLED_Clear();	//清屏
		g_ucWifi_iscon_flag = 0;
	}		
	
	xTaskCreate(MenuTask, "Menu", 128, NULL, 60, &g_xMenuHandle);
	xTaskCreate(K210Task, "K210", 128, NULL, 63, &g_xK210Handle);
	xTaskCreate(RC522Task,"RC522", 256, NULL, 65, &g_xRC522Handle);
	xTaskCreate(AS608Task,"AS608", 128, NULL, 68, &g_xAs608Handle);
	xTaskCreate(LightTask,"Light",64,NULL,50,&g_xLightHandle);
	xTaskCreate(WifiTask,"Wifi",128,NULL,62,&g_xWifiHandle);
	
	/* 创建监控任务 */
	xTaskCreate(MonitorTask, "Monitor", 128, NULL, 40, NULL);

	/* 初始化看门狗 */
	IWDG_Init();

	vTaskStartScheduler();
   while (1)
   {
	  
   }
}
