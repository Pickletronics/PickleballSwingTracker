#include "threads.h"

void UART_test(void *args) {
    // uint8_t len = 13;
    // char* data = "Hello World!\n";
    while (1) {
        // uart_write_bytes(UART_NUM_0, data, len);
        UARTprintf("test%69string", 69);
        vTaskDelay(100);
    }
}

void SPI_test(void *args) {
    while(1){
        uint8_t who_am_i = LSM6DSL_Read_WHOAMI();
        ESP_LOGI("LSM6DSL", "WHO_AM_I register: 0x%02X", who_am_i);
        vTaskDelay(100); 
    }   
}