#include "stm32f10x.h"                  // Device header
#include "PWM.h"

/**
  * 函    数：舵机初始化
  * 参    数：无
  * 返 回 值：无
  */
void Servo_Init(void)
{
	//*舵机周期20ms：1/(72000000/720) * 2000 * 1000 *//
	PWM_Init(720-1,2000-1);	//初始化舵机的底层PWM
}

/**
  * @brief  舵机角度转换为脉冲宽度
  * @param  angle: 角度
  * @retval 脉冲宽度
  */
u16 Servo_Angle_To_Pulse(u8 angle)
{
    // 0.5ms - 2.5ms对应0° - 180°
    // 20ms周期，ARR = 1999，PSC = 719
    // 脉冲宽度范围：50 - 250
    return (u16)(50 + (200 * angle / 180));
}

/**
  * @brief  控制舵机转动到指定角度
  * @param  angle: 角度
  * @retval 无
  */
void Servo_Control_Angle(u8 angle)
{
    u16 pulse = Servo_Angle_To_Pulse(angle);
    PWM_SetCompare1(pulse);
}

