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
    MPU9250_handle_t mpu;

    while(1){
        // printf("Who Am I: 0x%02X\n", MPU9250_read_WHOAMI());
        MPU9250_update(&mpu);
        printf("Accel.x = %d\n", mpu.accel.x);
        printf("Accel.y = %d\n", mpu.accel.y);
        printf("Accel.z = %d\n", mpu.accel.z);
        vTaskDelay(100); 
    }   
}