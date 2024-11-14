// button_SPI.c

/************************************Includes***************************************/

#include "button.h"
#include "threads.h"

/************************************Includes***************************************/

/********************************Public Variables***********************************/


/********************************Public Variables***********************************/

/********************************Public Functions***********************************/

void Button_Init() {
    esp_err_t ret;

    // Configure the button GPIO as input
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_NEGEDGE;      // Trigger on falling edge
    io_conf.pin_bit_mask = (1ULL << BUTTON_PIN);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;    // Enable internal pull-up
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;

    ret = gpio_config(&io_conf);
    if (ret != ESP_OK) {
        // Handle error if ISR service fails to install
        printf("Failed to configure GPIO %d: %s\n", BUTTON_PIN, esp_err_to_name(ret));
    }

    // Install ISR service and add ISR handler
    ret = gpio_install_isr_service(ESP_INTR_FLAG_EDGE);
    if (ret != ESP_OK) {
        // Handle error if ISR service fails to install
        printf("Failed to install ISR service: %s\n", esp_err_to_name(ret));
    }

    ret = gpio_isr_handler_add(BUTTON_PIN, Button_ISR, NULL);
    if (ret != ESP_OK) {
        // Handle error if ISR handler fails to add
        printf("Failed to add ISR handler: %s\n", esp_err_to_name(ret));
    }
}

int Button_Read() {
    return gpio_get_level(BUTTON_PIN);
}

/********************************Public Functions***********************************/