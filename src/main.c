// main.c

/************************************Includes***************************************/

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "threads.h"

/************************************Includes***************************************/

/********************************Public Variables***********************************/

SemaphoreHandle_t SPI_sem;
SemaphoreHandle_t SPIFFS_sem;
SemaphoreHandle_t Button_sem;
QueueHandle_t Button_queue;

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
    SPIFFS_sem = xSemaphoreCreateMutex();

    // Spawn threads
    xTaskCreatePinnedToCore(Sample_Sensor_task, "Sample_task", 4096, NULL, 1, NULL, 0);
    // xTaskCreatePinnedToCore(Process_Data_task, "Process_task", 4096, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(Button_task, "Button_task", 2048, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(Timer_task, "Timer_task", 2048, NULL, 1, NULL, 1);
    // xTaskCreate(SEM_test, "SEM_TEST", 2048, NULL, 1, NULL);
    xTaskCreatePinnedToCore(SPIFFS_Test_task, "SPIFFS_Test_task", 4096, NULL, 1, NULL, 0);

    // Plot threads
    // xTaskCreate(MPU9250_plot_accel, "Serial_Plot", 2048, NULL, 1, NULL);
    // xTaskCreate(MPU9250_plot_accel_mag, "Serial_Plot", 2048, NULL, 1, NULL);
    // xTaskCreate(MPU9250_plot_gyro, "Serial_Plot", 2048, NULL, 1, NULL);
}
