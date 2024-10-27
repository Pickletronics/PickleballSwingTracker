#include "UART.h"

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

void UARTprintf(const char *pcString, ...) {
    va_list vaArgP;
    // Start the varargs processing.
    va_start(vaArgP, pcString);

    // ASSERT(pcString != 0);

    uint32_t iter = 0;
    while(*pcString) {

        // find next format specifier -> %
        // stop when % or end of string
        while((pcString[iter] != '%') && (pcString[iter] != '\0')) {
            iter++;
        }

        // print portion of string leading up to %
        uart_write_bytes(UART_NUM, pcString, iter);
        pcString += iter;

        // check if fmt specifier found or end of string
        iter = 0;
        if (pcString[iter] == '%')
        {
            // skip %
            pcString++;

            uint32_t dig_count = 0;

            // check for digits
            while((pcString[iter] != '\0') && ((pcString[iter] >= '0') && (pcString[iter] <= '9'))) {
                int dig = pcString[iter]-'0';
                dig_count++;
                iter++;
            }

            // switch(*pcString++) {
            // }
        }
    }
    printf("exiting UARTprintf\n");

    // done with varargs
    va_end(vaArgP);
}