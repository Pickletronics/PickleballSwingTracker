// main.c

/************************************Includes***************************************/

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "threads.h"

/************************************Includes***************************************/

/********************************Public Variables***********************************/

SemaphoreHandle_t SPI_sem;
QueueHandle_t data_queue; 
SemaphoreHandle_t Button_sem;
// SemaphoreHandle_t dataDump_sem;

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
    SPIFFS_init(); 

    // Initialize semaphores
    SPI_sem = xSemaphoreCreateMutex();
    Button_sem = xSemaphoreCreateBinary();
    // dataDump_sem = xSemaphoreCreateBinary();
    
    // Initialize queues
    data_queue = xQueueCreate(100, sizeof(int16_t));

    // Spawn threads
    xTaskCreate(SPI_test, "SPI_TEST", 2048, NULL, 1, NULL);
    xTaskCreate(SEM_test, "UART_TEST", 2048, NULL, 1, NULL);
    xTaskCreate(Button_task, "Button_task", 2048, NULL, 1, NULL);
    xTaskCreate(DumpData_task, "DataDump_task", 4096, NULL, 1, NULL);

    // Plot threads
    // xTaskCreate(MPU9250_plot_accel, "Serial_Plot", 2048, NULL, 1, NULL);
    // xTaskCreate(MPU9250_plot_gyro, "Serial_Plot", 2048, NULL, 1, NULL);
}
