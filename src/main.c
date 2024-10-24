#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "threads.h"
#include "UART.h"
#include "SPI.h"
#include "LSM6DSL.h"

void app_main(void) {
     // Initialize modules
    UART_init();
    SPI_Init(); 
    LSM6DSL_Init(); 

    // Spawn threads
    xTaskCreate(UART_test, "UART_TEST", 2048, NULL, 1, NULL);
    xTaskCreate(SPI_test, "SPI_TEST", 2048, NULL, 1, NULL);
    
    // Do nothing!
    for (;;) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    while(1); 
}
