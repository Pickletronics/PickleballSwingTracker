// UART.c

/************************************Includes***************************************/

#include "UART.h"

/************************************Includes***************************************/

/********************************Public Variables***********************************/
/********************************Public Variables***********************************/

/********************************Public Functions***********************************/

void UART_init() {
    uart_config_t uart_config = {
        .baud_rate = CONFIG_MONITOR_BAUD,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    int intr_alloc_flags = 0;

    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_0, 1024 * 2, 0, 0, NULL, intr_alloc_flags));
    ESP_ERROR_CHECK(uart_param_config(UART_NUM_0, &uart_config));
}

void UART_write(const void* src, size_t size) {
    uart_write_bytes(UART_NUM, src, size);
}

/********************************Public Functions***********************************/