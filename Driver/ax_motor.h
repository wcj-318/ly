#ifndef __AX_MOTOR_H
#define __AX_MOTOR_H

#include "stm32f10x.h"

#define AX_MOTOR_PWM_MAX 7200

void AX_MOTOR_Init(void);
void AX_MOTOR_A_SetSpeed(int16_t speed);
void AX_MOTOR_B_SetSpeed(int16_t speed);
void AX_MOTOR_SetSideSpeed(int16_t left_speed, int16_t right_speed);

void Car_Forward(int16_t speed);
void Car_Backward(int16_t speed);
void Car_LeftShift(int16_t speed);
void Car_RightShift(int16_t speed);
void Car_LeftRotate(int16_t speed);
void Car_RightRotate(int16_t speed);
void Car_Drift(int16_t speed);
void Car_Stop(void);

#endif
