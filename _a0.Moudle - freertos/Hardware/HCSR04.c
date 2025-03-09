#include "stm32f10x.h"                  // Device header

#define HCSR04_PORT     GPIOB
#define HCSR04_CLK      RCC_APB2Periph_GPIOB
#define HCSR04_TRIG     GPIO_Pin_9
#define HCSR04_ECHO     GPIO_Pin_8

#define TRIG_H  GPIO_SetBits(HCSR04_PORT,HCSR04_TRIG)
#define TRIG_L	 GPIO_ResetBits(HCSR04_PORT,HCSR04_TRIG)

uint16_t g_usMs_HcCount = 0;//ms计数
uint64_t time=0;			//声明变量，用来计时
uint64_t time_end=0;		//声明变量，存储回波信号时间

uint8_t ECHO_Reci(void){
	return GPIO_ReadInputDataBit(HCSR04_PORT,HCSR04_ECHO);
}  

static void Delay_us(uint16_t time)  //延时函数
{ 
    uint16_t i,j;
    for(i=0;i<time;i++)
          for(j=0;j<9;j++);
}


void Hcsr04Init(void)
{  
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStruc;     //生成用于定时器设置的结构体
	GPIO_InitTypeDef GPIO_InitStruc;
	RCC_APB2PeriphClockCmd(HCSR04_CLK, ENABLE);
	
	//IO初始化
	GPIO_InitStruc.GPIO_Pin =HCSR04_TRIG;       //发送电平引脚
	GPIO_InitStruc.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruc.GPIO_Mode = GPIO_Mode_Out_PP;//推挽输出
	GPIO_Init(HCSR04_PORT, &GPIO_InitStruc);
	GPIO_ResetBits(HCSR04_PORT,HCSR04_TRIG);

	GPIO_InitStruc.GPIO_Pin =   HCSR04_ECHO;     //返回电平引脚
	GPIO_InitStruc.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_Init(HCSR04_PORT, &GPIO_InitStruc);  
	GPIO_ResetBits(HCSR04_PORT,HCSR04_ECHO);    

	//Timer初始化
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);   //使能对应RCC时钟
	TIM_InternalClockConfig(TIM4);								//设置TIM3使用内部时钟
	TIM_DeInit(TIM4);
	TIM_TimeBaseStruc.TIM_Period = (10-1); //设置在下一个更新事件装入活动的自动重装载寄存器周期的值         计数到1000为1ms
	TIM_TimeBaseStruc.TIM_Prescaler =(72-1); //设置用来作为TIMx时钟频率除数的预分频值  1M的计数频率 1US计数
	TIM_TimeBaseStruc.TIM_ClockDivision=TIM_CKD_DIV1;//不分频
	TIM_TimeBaseStruc.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStruc); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位         

	TIM_ClearFlag(TIM4, TIM_FLAG_Update);   //清除更新中断，免得一打开中断立即产生中断
	TIM_ITConfig(TIM4,TIM_IT_Update,ENABLE);    //打开定时器更新中断
	
	//中断配置
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;             
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;         
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;        
	NVIC_Init(&NVIC_InitStructure);	
	
	TIM_Cmd(TIM4,DISABLE);       
}

uint32_t sonar_mm(void )
{
	uint32_t Distance,Distance_mm = 0;
	TRIG_H;						//输出高电平
	Delay_us(15);										//延时15微秒
	TRIG_L;						//输出低电平
	while(ECHO_Reci());		//等待低电平结束
	time=0;												//计时清零
	while(ECHO_Reci());		//等待高电平结束
	time_end=time;										//记录结束时的时间
	if(time_end/100<38)									//判断是否小于38毫秒，大于38毫秒的就是超时，直接调到下面返回0
	{
		Distance=(time_end*346)/2;						//计算距离，25°C空气中的音速为346m/s
		Distance_mm=Distance/100;						//因为上面的time_end的单位是10微秒，所以要得出单位为毫米的距离结果，还得除以100
	}
	return Distance_mm;									//返回测距结果

}

float sonar(void)										//测距并返回单位为米的距离结果
{
	uint32_t Distance,Distance_mm = 0;
	float Distance_m=0;
	TRIG_H;					//输出高电平
	Delay_us(15);
	TRIG_L;					//输出低电平
	while(ECHO_Reci() == 0);
	time=0;
	while(ECHO_Reci() == 1);
	time_end=time;
	if(time_end/100<38)
	{
		Distance=(time_end*346)/2;
		Distance_mm=Distance/100;
		Distance_m=Distance_mm/1000;
	}
	return Distance_m;
}

//定时器4中断服务程序
void TIM4_IRQHandler(void)   //TIM4中断
{
	if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)  //检查TIM4更新中断发生与否
	{
		time++;
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update  );  //清除TIMx更新中断标志 			 
	}
}

