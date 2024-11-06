// MPU9250_SPI.c

/************************************Includes***************************************/

#include "BUTTON.h"
#include "threads.h"

/************************************Includes***************************************/

/********************************Public Variables***********************************/


/********************************Public Variables***********************************/

/********************************Public Functions***********************************/

void Button_Init() {
    // Configure the button GPIO as input
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_NEGEDGE;      // Trigger on falling edge
    io_conf.pin_bit_mask = (1ULL << BUTTON_PIN);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;    // Enable internal pull-up
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpio_config(&io_conf);

    // Install ISR service and add ISR handler
    gpio_install_isr_service(ESP_INTR_FLAG_EDGE | ESP_INTR_FLAG_IRAM);
    gpio_isr_handler_add(BUTTON_PIN, buttonISR, NULL);
}

/********************************Public Functions***********************************/

/**********************************Test Threads*************************************/


/**********************************Test Threads*************************************/