#include <stdint.h>
#include "UART.h"

void app_main(void) {
    UART_init();
    uint8_t len = 13;
    char* data = "Hello World!\n";
    while (1) {
        uart_write_bytes(UART_NUM_0, data, len);
        vTaskDelay(100);
    }
    
}
