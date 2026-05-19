#ifndef __AX_UART_H
#define __AX_UART_H

#include "stm32f10x.h"

void AX_UART_Init(uint32_t baud);

void AX_BLUETOOTH_Init(uint32_t baud);
uint8_t AX_BLUETOOTH_GetCommand(uint8_t *command);
void AX_BLUETOOTH_IRQHandler(void);

#endif
