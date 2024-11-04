// threads.c

/************************************Includes***************************************/

#include "threads.h"

/************************************Includes***************************************/

/********************************Public Variables***********************************/
/********************************Public Variables***********************************/

/********************************Public Functions***********************************/
/********************************Public Functions***********************************/

/***********************************Test Threads************************************/

void SEM_test(void *args) {
    TickType_t start, end;

    while (1) {
        if (xSemaphoreTake(SPI_sem, 0)) {
            start = xTaskGetTickCount();

            // simulate variable work
            vTaskDelay(rand() % 101);

            // release semaphore
            xSemaphoreGive(SPI_sem);
            end = xTaskGetTickCount();

            printf("\nSemaphore held for %.2f seconds\n", ((float)(end-start))/configTICK_RATE_HZ);
        }
        else {}

        vTaskDelay(1);
    }
}

void SPI_test(void *args) {
    while(1){
        // try to acquire semaphore for 10 ticks
        if (xSemaphoreTake(SPI_sem, 10)) {

            // read MPU9250 over SPI
            printf("\n");
            printf("Who Am I: 0x%02X\n", MPU9250_read_WHOAMI());
            MPU9250_update();

            // release semaphore
            xSemaphoreGive(SPI_sem);

            // print data
            printf("\n");
            printf("Accel.x = %d\n", mpu.accel.x);
            printf("Accel.y = %d\n", mpu.accel.y);
            printf("Accel.z = %d\n", mpu.accel.z);
            printf("\n");
            printf("Gyro.x = %d\n", mpu.gyro.x);
            printf("Gyro.y = %d\n", mpu.gyro.y);
            printf("Gyro.z = %d\n", mpu.gyro.z);
        }
        else {}

        vTaskDelay(1); 
    }   
}

/***********************************Test Threads************************************/
