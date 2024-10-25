// SPI.c
// Date Created: 2024-10-24
// Date Updated: 2024-10-24
// Defines for ESP32 SPI functions

/************************************Includes***************************************/

#include "SPI.h"
#include <string.h>

/********************************Public Functions***********************************/
// Global handle for SPI device
static spi_device_handle_t spi_handle;

esp_err_t SPI_Init(){
    // Configures the spi bus
    spi_bus_config_t buscfg = {
        .miso_io_num = PIN_NUM_MISO, 
        .mosi_io_num = PIN_NUM_MOSI, 
        .sclk_io_num = PIN_NUM_CLK, 
        .quadwp_io_num = -1,        
        .quadhd_io_num = -1,        
        .max_transfer_sz = 4096     
    };

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 1000000,              // Set to appropriate frequency
        .mode = 3,                              // SPI mode 3 for LSM6DSL
        .spics_io_num = PIN_NUM_CS,         // CS pin
        .queue_size = 7,                        // Queue size for transactions
    };

    esp_err_t ret = spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        printf("Failed to initialize SPI bus: %s\n", esp_err_to_name(ret));
    }
    ret = spi_bus_add_device(SPI2_HOST, &devcfg, &spi_handle);
    if (ret != ESP_OK) {
        printf("Failed to add device to SPI bus\n");
        return ret;
    }
    // Configure Chip Select
    gpio_set_direction(PIN_NUM_CS, GPIO_MODE_OUTPUT);
    SPI_Deselect();
    return ret;
}

esp_err_t SPI_Select(){
    return gpio_set_level(PIN_NUM_CS, 0);
}

esp_err_t SPI_Deselect(){
    return gpio_set_level(PIN_NUM_CS, 1);
}

esp_err_t SPI_Write(uint8_t *data, size_t length){
    spi_transaction_t to_write = {
        .length = length * 8,
        .tx_buffer = data
    };
    return spi_device_transmit(spi_handle, &to_write);
}

esp_err_t SPI_Read(uint8_t *data, size_t length){
    spi_transaction_t to_read = {
        .length = length * 8,
        .rx_buffer = data
    };
    return spi_device_transmit(spi_handle, &to_read);
}
