// main.c

/************************************Includes***************************************/

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "threads.h"

/************************************Includes***************************************/

/********************************Public Variables***********************************/

SemaphoreHandle_t SPI_sem;
SemaphoreHandle_t UART_sem;
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
    MPU9250_Init();
    BLE_Start(); 
    SPIFFS_init(); 
    Button_Init();

    // Initialize semaphores
    SPI_sem = xSemaphoreCreateMutex();
    Button_sem = xSemaphoreCreateBinary();
    UART_sem = xSemaphoreCreateMutex();
    SPIFFS_sem = xSemaphoreCreateMutex();

    // Spawn threads
    xTaskCreatePinnedToCore(Play_Session_task, "Session_task", 4096, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(Button_task, "Button_task", 2048, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(Button_Manager_task, "Button_Manager_task", 2048, NULL, 1, NULL, 1);
    // xTaskCreatePinnedToCore(SPIFFS_Test_task, "SPIFFS_Test_task", 4096, NULL, 1, NULL, 0);

    // Plot threads
    // xTaskCreate(MPU9250_plot_accel, "Serial_Plot", 2048, NULL, 1, NULL);
    // xTaskCreate(MPU9250_plot_accel_mag, "Serial_Plot", 2048, NULL, 1, NULL);
    // xTaskCreate(MPU9250_plot_gyro, "Serial_Plot", 2048, NULL, 1, NULL);
}
