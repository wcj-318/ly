#include "ax_motor.h"

#define LF_IN1_PORT GPIOB
#define LF_IN1_PIN  GPIO_Pin_12
#define LF_IN2_PORT GPIOB
#define LF_IN2_PIN  GPIO_Pin_13

#define LR_IN1_PORT GPIOB
#define LR_IN1_PIN  GPIO_Pin_14
#define LR_IN2_PORT GPIOB
#define LR_IN2_PIN  GPIO_Pin_15

#define RF_IN1_PORT GPIOA
#define RF_IN1_PIN  GPIO_Pin_4
#define RF_IN2_PORT GPIOA
#define RF_IN2_PIN  GPIO_Pin_5

#define RR_IN1_PORT GPIOB
#define RR_IN1_PIN  GPIO_Pin_8
#define RR_IN2_PORT GPIOB
#define RR_IN2_PIN  GPIO_Pin_9

static uint16_t AX_MOTOR_LimitAbs(int16_t speed)
{
	uint16_t value;

	value = (speed < 0) ? (uint16_t)(-speed) : (uint16_t)speed;
	return (value > AX_MOTOR_PWM_MAX) ? AX_MOTOR_PWM_MAX : value;
}

static void AX_MOTOR_SetDir(GPIO_TypeDef *in1_port, uint16_t in1_pin,
							GPIO_TypeDef *in2_port, uint16_t in2_pin,
							int8_t dir)
{
	if (dir > 0)
	{
		GPIO_ResetBits(in2_port, in2_pin);
		GPIO_SetBits(in1_port, in1_pin);
	}
	else if (dir < 0)
	{
		GPIO_ResetBits(in1_port, in1_pin);
		GPIO_SetBits(in2_port, in2_pin);
	}
	else
	{
		GPIO_ResetBits(in1_port, in1_pin);
		GPIO_ResetBits(in2_port, in2_pin);
	}
}

static void Car_SetWheelDir(int8_t lf, int8_t lr, int8_t rf, int8_t rr)
{
	AX_MOTOR_SetDir(LF_IN1_PORT, LF_IN1_PIN, LF_IN2_PORT, LF_IN2_PIN, lf);
	AX_MOTOR_SetDir(LR_IN1_PORT, LR_IN1_PIN, LR_IN2_PORT, LR_IN2_PIN, lr);
	AX_MOTOR_SetDir(RF_IN1_PORT, RF_IN1_PIN, RF_IN2_PORT, RF_IN2_PIN, rf);
	AX_MOTOR_SetDir(RR_IN1_PORT, RR_IN1_PIN, RR_IN2_PORT, RR_IN2_PIN, rr);
}

void AX_MOTOR_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_OCInitTypeDef TIM_OCInitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 |
								  GPIO_Pin_12 | GPIO_Pin_13 |
								  GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	TIM_TimeBaseStructure.TIM_Period = AX_MOTOR_PWM_MAX - 1;
	TIM_TimeBaseStructure.TIM_Prescaler = 0;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OCInitStructure.TIM_Pulse = 0;

	TIM_OC1Init(TIM1, &TIM_OCInitStructure);
	TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);

	TIM_OC4Init(TIM1, &TIM_OCInitStructure);
	TIM_OC4PreloadConfig(TIM1, TIM_OCPreload_Enable);

	TIM_CtrlPWMOutputs(TIM1, ENABLE);
	TIM_Cmd(TIM1, ENABLE);

	Car_Stop();
}

void AX_MOTOR_A_SetSpeed(int16_t speed)
{
	uint16_t temp;

	if (speed > 0)
	{
		GPIO_ResetBits(GPIOB, GPIO_Pin_13);
		GPIO_SetBits(GPIOB, GPIO_Pin_12);
		temp = AX_MOTOR_LimitAbs(speed);
	}
	else if (speed < 0)
	{
		GPIO_ResetBits(GPIOB, GPIO_Pin_12);
		GPIO_SetBits(GPIOB, GPIO_Pin_13);
		temp = AX_MOTOR_LimitAbs(speed);
	}
	else
	{
		GPIO_ResetBits(GPIOB, GPIO_Pin_12);
		GPIO_ResetBits(GPIOB, GPIO_Pin_13);
		temp = 0;
	}

	TIM_SetCompare1(TIM1, temp);
}

void AX_MOTOR_B_SetSpeed(int16_t speed)
{
	uint16_t temp;

	if (speed > 0)
	{
		GPIO_ResetBits(GPIOB, GPIO_Pin_15);
		GPIO_SetBits(GPIOB, GPIO_Pin_14);
		temp = AX_MOTOR_LimitAbs(speed);
	}
	else if (speed < 0)
	{
		GPIO_ResetBits(GPIOB, GPIO_Pin_14);
		GPIO_SetBits(GPIOB, GPIO_Pin_15);
		temp = AX_MOTOR_LimitAbs(speed);
	}
	else
	{
		GPIO_ResetBits(GPIOB, GPIO_Pin_14);
		GPIO_ResetBits(GPIOB, GPIO_Pin_15);
		temp = 0;
	}

	TIM_SetCompare4(TIM1, temp);
}

void AX_MOTOR_SetSideSpeed(int16_t left_speed, int16_t right_speed)
{
	TIM_SetCompare1(TIM1, AX_MOTOR_LimitAbs(left_speed));
	TIM_SetCompare4(TIM1, AX_MOTOR_LimitAbs(right_speed));
}

void Car_Forward(int16_t speed)
{
	Car_SetWheelDir(1, 1, 1, 1);
	AX_MOTOR_SetSideSpeed(speed, speed);
}

void Car_Backward(int16_t speed)
{
	Car_SetWheelDir(-1, -1, -1, -1);
	AX_MOTOR_SetSideSpeed(speed, speed);
}

void Car_LeftShift(int16_t speed)
{
	Car_SetWheelDir(-1, 1, 1, -1);
	AX_MOTOR_SetSideSpeed(speed, speed);
}

void Car_RightShift(int16_t speed)
{
	Car_SetWheelDir(1, -1, -1, 1);
	AX_MOTOR_SetSideSpeed(speed, speed);
}

void Car_LeftRotate(int16_t speed)
{
	Car_SetWheelDir(-1, -1, 1, 1);
	AX_MOTOR_SetSideSpeed(speed, speed);
}

void Car_RightRotate(int16_t speed)
{
	Car_SetWheelDir(1, 1, -1, -1);
	AX_MOTOR_SetSideSpeed(speed, speed);
}

void Car_Drift(int16_t speed)
{
	Car_SetWheelDir(1, 1, 1, 1);
	AX_MOTOR_SetSideSpeed(speed, speed / 3);
}

void Car_Stop(void)
{
	Car_SetWheelDir(0, 0, 0, 0);
	AX_MOTOR_SetSideSpeed(0, 0);
}
