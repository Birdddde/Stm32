#include "stm32f10x.h"
#include <stdio.h>
#include "freertos.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "oled.h"
#include "delay.h"

#define KEY_PIN GPIO_Pin_1  // 假设按键连接在GPIO_PIN_0

xTimerHandle g_xKeyScanTimer;  // 定时器句柄
    static uint8_t lastState = 1;  // 记录上次按键状态
    static uint8_t currentState = 1;
	 static uint8_t cnt=0;
	 
void Key_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);//使能GPIOA时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);//使能AFIO钟
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = KEY_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource1);//配置外部中断线
	
	EXTI_InitTypeDef EXTI_InitStructure;
	EXTI_InitStructure.EXTI_Line=EXTI_Line1;//指定外部中断线
	EXTI_InitStructure.EXTI_LineCmd=ENABLE;//使能该中断线
	EXTI_InitStructure.EXTI_Mode=EXTI_Mode_Interrupt;//设置外部中断模式为中断模式（非事件模式）。
	EXTI_InitStructure.EXTI_Trigger=EXTI_Trigger_Falling;//触发条件
	EXTI_Init(&EXTI_InitStructure);//初始化外部中断
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//配置NVIC（嵌套向量中断控制器）的优先级分组
	
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel=EXTI1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;//设置中断使能状态
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;//设置抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=1;//设置子优先级
	NVIC_Init(&NVIC_InitStructure);//初始化NVIC中断源
}
// 按键扫描函数
void vKeyScan(void)
{

    currentState = GPIO_ReadInputDataBit(GPIOB,KEY_PIN);  // 读取按键状态

    if (currentState == 0 && lastState == 1) {
        // 按键被按下
		 cnt++;
		 OLED_ShowNum(3,1,cnt,2);
        OLED_ShowString(2, 2, "Button Pressed");
    } else if (currentState == 0 && lastState == 1) {
        // 按键被释放
    }

    lastState = currentState;  // 更新上次按键状态
}

// 定时器回调函数
void vKeyTimerCallback(xTimerHandle pxTimer)
{
    vKeyScan();  // 每次定时器回调时扫描按键
}

// 创建定时器
void Timer_Create(void)
{
    const TickType_t xTimerPeriod = pdMS_TO_TICKS(60);  // 定时器周期20ms

    g_xKeyScanTimer = xTimerCreate("KeyScanTimer",  // 定时器名称
                                    xTimerPeriod,    // 定时器周期
                                    pdTRUE,          // 周期性定时器
                                    NULL,             // 定时器 ID
                                    vKeyTimerCallback); // 回调函数

}

int main(void)
{
	OLED_Init();
   Key_Init();  // 初始化GPIO按键
	
	OLED_ShowString(1,1,"helo");
    // 创建定时器
   Timer_Create();

    // 启动任务调度器
   vTaskStartScheduler();
}

void EXTI1_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line1)==SET)
	{
		if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_1)==0)
		{
			xTimerResetFromISR(g_xKeyScanTimer,NULL);
		}
	}
	EXTI_ClearFlag(EXTI_Line1);//清除中断标志位
}
