#include "driver/uart.h"

#define UART_NUM        UART_NUM_0

void UART_init();
void UARTprintf(const char *pcString, ...);