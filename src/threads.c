#include "threads.h"

void UART_test(void *args) {
    uint8_t len = 13;
    char* data = "Hello World!\n";
    while (1) {
        uart_write_bytes(UART_NUM_0, data, len);
        vTaskDelay(100);
    }
}

void UART_test2(void *args) {
    uint8_t len = 18;
    char* data = "Hello evil World!\n";
    while (1) {
        uart_write_bytes(UART_NUM_0, data, len);
        vTaskDelay(50);
    }
}