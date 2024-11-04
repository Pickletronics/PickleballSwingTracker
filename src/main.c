#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "threads.h"

SemaphoreHandle_t SPI_sem;

/* Main */
/// Used to initialize peripherals and spawn threads
/// ESP-IDF starts FreeRTOS automatically when function returns
void app_main(void) {
    // Initialize modules
    UART_init();
    SPI_Init(); 
    MPU9250_Init();

    // Initialize semaphores
    SPI_sem = xSemaphoreCreateMutex();

    // Spawn threads
    xTaskCreate(SPI_test, "SPI_TEST", 2048, NULL, 1, NULL);
    xTaskCreate(SEM_test, "UART_TEST", 2048, NULL, 1, NULL);

    // Plot threads
    // xTaskCreate(MPU9250_plot_accel, "Serial_Plot", 2048, NULL, 1, NULL);
    // xTaskCreate(MPU9250_plot_gyro, "Serial_Plot", 2048, NULL, 1, NULL);
}
