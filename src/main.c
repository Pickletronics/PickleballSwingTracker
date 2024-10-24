#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "threads.h"
#include "UART.h"

void app_main(void) {
    // Initialize modules
    UART_init();

    // Spawn threads
    xTaskCreate(UART_test, "UART_TEST", 2048, NULL, 1, NULL);
    xTaskCreate(UART_test2, "UART_TEST2", 2048, NULL, 1, NULL);
    
    // Do nothing!
    for (;;) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
