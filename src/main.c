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
SemaphoreHandle_t Button_sem;

/********************************Public Variables***********************************/

/* Main */
/// Used to initialize peripherals and spawn threads
/// ESP-IDF starts FreeRTOS automatically when function returns
void app_main(void) {
    // Initialize modules
    SPI_Init(); 
    UART_init();
    Button_Init();
    MPU9250_Init();
    BLE_Start(); 

    // Initialize semaphores
    SPI_sem = xSemaphoreCreateMutex();
    data_queue = xQueueCreate(9, sizeof(int16_t));
    Button_sem = xSemaphoreCreateBinary();

    // Spawn threads
    xTaskCreate(SPI_test, "SPI_TEST", 2048, NULL, 1, NULL);
    xTaskCreate(SEM_test, "UART_TEST", 2048, NULL, 1, NULL);
    xTaskCreate(Button_task, "button_task", 2048, NULL, 1, NULL);

    // Plot threads
    // xTaskCreate(MPU9250_plot_accel, "Serial_Plot", 2048, NULL, 1, NULL);
    // xTaskCreate(MPU9250_plot_gyro, "Serial_Plot", 2048, NULL, 1, NULL);
}
