#include "UART.h"
#include "SPI.h"
#include "MPU9250.h"

extern MPU9250_handle_t mpu;

void UART_test(void *args);
void SPI_test(void *args); 
void Serial_Plot(void *args);