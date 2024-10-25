#include "threads.h"

void UART_test(void *args) {
    uint8_t len = 13;
    char* data = "Hello World!\n";
    while (1) {
        uart_write_bytes(UART_NUM_0, data, len);
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

void ACCEL_test(void *args){
    while (1) {
        // Read each axis
        int16_t accel_x = read_accel_axis(ACCEL_X_L, ACCEL_X_H);
        int16_t accel_y = read_accel_axis(ACCEL_Y_L, ACCEL_Y_H);
        int16_t accel_z = read_accel_axis(ACCEL_Z_L, ACCEL_Z_H);

        // Print accelerometer data in CSV format
        uart_write_bytes(UART_NUM_0, (char*)accel_x, 16);
        uart_write_bytes(UART_NUM_0, (char*)accel_y, 16);
        uart_write_bytes(UART_NUM_0, (char*)accel_z, 16);
    }
}
