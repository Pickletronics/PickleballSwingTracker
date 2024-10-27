#include "threads.h"

void UART_test(void *args) {
    uint8_t len = 13;
    char* data = "Hello World!\n";
    while (1) {
        UART_write(data, len);
        vTaskDelay(100);
    }
}

void SPI_test(void *args) {
    while(1){
        // printf("Who Am I: 0x%02X\n", MPU9250_read_WHOAMI());
        
        MPU9250_update();

        printf("\n");
        printf("Accel.x = %d\n", mpu.accel.x);
        printf("Accel.y = %d\n", mpu.accel.y);
        printf("Accel.z = %d\n", mpu.accel.z);

        printf("\n");
        printf("Gyro.x = %d\n", mpu.gyro.x);
        printf("Gyro.y = %d\n", mpu.gyro.y);
        printf("Gyro.z = %d\n", mpu.gyro.z);

        vTaskDelay(100); 
    }   
}

void Serial_Plot(void *args) {
    while (1) {
        MPU9250_plot_accel();
        vTaskDelay(1);
    }
    
}