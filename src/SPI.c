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

void SPI_Init(){
    // Configures the spi bus
    spi_bus_config_t buscfg = {
    .miso_io_num = PIN_NUM_MISO, 
    .mosi_io_num = PIN_NUM_MOSI, 
    .sclk_io_num = PIN_NUM_CLK, 
    .quadwp_io_num = -1,        
    .quadhd_io_num = -1,        
    .max_transfer_sz = 4096     
    };
    spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);

    // Configure the LSM6DSR device for SPI 
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 5 * 1000 * 1000, 
        .mode = 3,                         
        .spics_io_num = PIN_NUM_CS,         
        .queue_size = 7
    };
    spi_bus_add_device(SPI2_HOST, &devcfg, &spi_handle);
}

esp_err_t SPI_Transmit(uint8_t* tx_data, uint8_t* rx_data, size_t len){
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length = len * 8;            
    t.tx_buffer = tx_data;
    t.rx_buffer = rx_data; 

    // Transmit and recieve the data
    return spi_device_transmit(spi_handle, &t);
}