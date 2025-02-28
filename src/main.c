// main.c

/************************************Includes***************************************/

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "button.h"

/************************************Includes***************************************/

/********************************Public Variables***********************************/

// SemaphoreHandle_t SPI_sem;
// SemaphoreHandle_t UART_sem;
// SemaphoreHandle_t SPIFFS_sem;

/********************************Public Variables***********************************/

/* Main */
/// Used to initialize peripherals and spawn threads
/// ESP-IDF starts FreeRTOS automatically when function returns
void app_main(void) {
    // Initialize modules
    // UART_init();
    // LED_init();
    // SPI_Init(); 
    // MPU9250_Init();
    // SPIFFS_init(); 

    const gpio_num_t LED_PIN = 15;
    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_PIN, 1);

    Button_Init();

    while (1)
    {
        // gpio_set_level(LED_PIN, 1);
        // vTaskDelay(1000);
        // gpio_set_level(LED_PIN, 0);
        printf("main running\n");
        vTaskDelay(1000);
    }
    

    // Initialize semaphores
    // SPI_sem = xSemaphoreCreateMutex();
    // UART_sem = xSemaphoreCreateMutex();
    // SPIFFS_sem = xSemaphoreCreateMutex();

    // Spawn threads
    // xTaskCreatePinnedToCore(Play_Session_task, "Session_task", 4096, NULL, 1, NULL, 0);
    // xTaskCreatePinnedToCore(FSM_task, "FSM_task", 4096, NULL, 1, NULL, 1);

    // Plot threads
    // xTaskCreate(MPU9250_plot_accel, "Serial_Plot", 2048, NULL, 1, NULL);
    // xTaskCreate(MPU9250_plot_accel_mag, "Serial_Plot", 2048, NULL, 1, NULL);
    // xTaskCreate(MPU9250_plot_gyro, "Serial_Plot", 2048, NULL, 1, NULL);
}
