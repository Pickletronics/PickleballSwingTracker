// main.c

/************************************Includes***************************************/

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "threads.h"
#include "BLE.h"

/************************************Includes***************************************/

/********************************Public Variables***********************************/

SemaphoreHandle_t SPI_sem;
QueueHandle_t data_queue; 

/********************************Public Variables***********************************/

/* Main */
/// Used to initialize peripherals and spawn threads
/// ESP-IDF starts FreeRTOS automatically when function returns
void app_main(void) {
    // Initialize modules
    UART_init();
    SPI_Init(); 
    MPU9250_Init();
    BLE_Start(); 

    // Initialize semaphores
    SPI_sem = xSemaphoreCreateMutex();
    data_queue = xQueueCreate(3, sizeof(int16_t));

    // Spawn threads
    xTaskCreate(SPI_test, "SPI_TEST", 2048, NULL, 1, NULL);
    xTaskCreate(SEM_test, "UART_TEST", 2048, NULL, 1, NULL);

    // Plot threads
    // xTaskCreate(MPU9250_plot_accel, "Serial_Plot", 2048, NULL, 1, NULL);
    // xTaskCreate(MPU9250_plot_gyro, "Serial_Plot", 2048, NULL, 1, NULL);
}
