// SPI.c

/************************************Includes***************************************/

#include "SPI.h"
#include <string.h>

/************************************Includes***************************************/

/********************************Public Variables***********************************/

// Global handle for SPI device
spi_device_handle_t spi_handle;

/********************************Public Variables***********************************/

/********************************Public Functions***********************************/

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

    esp_err_t ret = spi_bus_initialize(SPI_NUM, &buscfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        printf("Failed to initialize SPI bus: %s\n", esp_err_to_name(ret));
    }
    
    return ret;
}

esp_err_t SPI_Select(uint8_t CS_num){
    return gpio_set_level(CS_num, 0);
}

esp_err_t SPI_Deselect(uint8_t CS_num){
    return gpio_set_level(CS_num, 1);
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

esp_err_t SPI_Write_Read(uint8_t reg, uint8_t *data, size_t length) {
    // Transmit and receive buffer
    uint8_t tx_buffer[1 + length];  // 1 byte for address, rest for dummy bytes
    uint8_t rx_buffer[1 + length]; // 1 byte for response to address, rest for data

    tx_buffer[0] = reg | 0x80;  // Set the read bit (MSB = 1)
    memset(&tx_buffer[1], 0xFF, length); // Fill dummy bytes

    // Configure transaction
    spi_transaction_t transaction = {
        .length = (1 + length) * 8, // Total number of bits
        .tx_buffer = tx_buffer,
        .rx_buffer = rx_buffer,
    };

    // Perform SPI transaction
    esp_err_t ret = spi_device_transmit(spi_handle, &transaction);
    if (ret == ESP_OK) {
        memcpy(data, &rx_buffer[1], length); // Skip the first byte (address response)
    }
    return ret;
}


/********************************Public Functions***********************************/