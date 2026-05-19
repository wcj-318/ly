#include "stm32f10x.h"
#include <stdio.h>

#include "ax_delay.h"
#include "ax_encoder.h"
#include "ax_motor.h"
#include "ax_sys.h"
#include "ax_uart.h"
#include "ax_vin.h"

#define CAR_DEFAULT_SPEED 2000
#define CAR_SPEED_STEP    500

static int16_t car_speed = CAR_DEFAULT_SPEED;
static int16_t encoder1, encoder2;
static uint16_t adc_value;

static void Car_HandleCommand(uint8_t command)
{
	switch (command)
	{
	case 'F':
		Car_Forward(car_speed);
		break;
	case 'B':
		Car_Backward(car_speed);
		break;
	case 'L':
		Car_LeftShift(car_speed);
		break;
	case 'R':
		Car_RightShift(car_speed);
		break;
	case 'G':
		Car_LeftRotate(car_speed);
		break;
	case 'H':
		Car_RightRotate(car_speed);
		break;
	case 'S':
		Car_Stop();
		break;
	case 'A':
		if (car_speed + CAR_SPEED_STEP <= AX_MOTOR_PWM_MAX)
		{
			car_speed += CAR_SPEED_STEP;
		}
		break;
	case 'D':
		Car_Drift(car_speed);
		break;
	case '\r':
	case '\n':
		break;
	default:
		Car_Stop();
		break;
	}
}

int main(void)
{
	uint8_t command;
	uint8_t sample_tick = 0;

	AX_DELAY_Init();
	AX_UART_Init(115200);

	printf("\r\nAndroid BLE Mecanum Car\r\n");
	printf("JDY-23 BLE UART: USART3 PB10(TX) PB11(RX), 9600bps\r\n");

	AX_JTAG_Set(JTAG_SWD_DISABLE);
	AX_JTAG_Set(SWD_ENABLE);

	AX_MOTOR_Init();
	AX_ENCODER_A_Init();
	AX_ENCODER_B_Init();
	AX_VIN_Init();
	AX_BLUETOOTH_Init(9600);

	Car_Stop();

	while (1)
	{
		if (AX_BLUETOOTH_GetCommand(&command))
		{
			Car_HandleCommand(command);
			printf("CMD:%c SPEED:%d\r\n", command, car_speed);
		}

		if (++sample_tick >= 10)
		{
			sample_tick = 0;

			encoder1 = (int16_t)AX_ENCODER_A_GetCounter();
			encoder2 = (int16_t)AX_ENCODER_B_GetCounter();
			AX_ENCODER_A_SetCounter(0);
			AX_ENCODER_B_SetCounter(0);

			adc_value = AX_VIN_GetVol_X100();
			printf("EA:%d EB:%d VIN:%d\r\n", encoder1, encoder2, adc_value);
		}

		AX_Delayms(10);
	}
}
