// main.c

/************************************Includes***************************************/

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "play_session.h"
#include "FSM.h"

/************************************Includes***************************************/

/********************************Public Variables***********************************/

SemaphoreHandle_t SPI_sem;
SemaphoreHandle_t UART_sem;
SemaphoreHandle_t SPIFFS_sem;

/********************************Public Variables***********************************/

/* Main */
/// Used to initialize peripherals and spawn threads
/// ESP-IDF starts FreeRTOS automatically when function returns
void app_main(void) {
    // Initialize modules
    UART_init();
    LED_init();
    SPI_Init(); 
    MPU9250_Init();
    SPIFFS_init(); 
    Button_Init();

    // Initialize semaphores
    SPI_sem = xSemaphoreCreateMutex();
    UART_sem = xSemaphoreCreateMutex();
    SPIFFS_sem = xSemaphoreCreateMutex();

    // Spawn threads
    // xTaskCreatePinnedToCore(Play_Session_task, "Session_task", 4096, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(FSM_task, "FSM_task", 4096, NULL, 1, NULL, 1);

    // Plot threads
    // xTaskCreate(MPU9250_plot_accel, "Serial_Plot", 2048, NULL, 1, NULL);
    // xTaskCreate(MPU9250_plot_accel_mag, "Serial_Plot", 2048, NULL, 1, NULL);
    // xTaskCreate(MPU9250_plot_gyro, "Serial_Plot", 2048, NULL, 1, NULL);
}
