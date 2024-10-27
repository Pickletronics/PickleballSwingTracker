#include "driver/uart.h"

#define UART_NUM                UART_NUM_0

void UART_init();
void UART_write(const void* src, size_t size);