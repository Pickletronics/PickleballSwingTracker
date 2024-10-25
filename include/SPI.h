// SPI.h
// Date Created: 2024-10-24
// Date Updated: 2024-10-24
// SPI header file

#ifndef SPI_H_
#define SPI_H_

/************************************Includes***************************************/

#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"
/************************************Includes***************************************/

/*************************************Defines***************************************/

// SPI pins for the ESP32
#define PIN_NUM_MISO        GPIO_NUM_19
#define PIN_NUM_MOSI        GPIO_NUM_23
#define PIN_NUM_CLK         GPIO_NUM_18
#define PIN_NUM_CS          GPIO_NUM_5

/*************************************Defines***************************************/

esp_err_t SPI_Init(void);
esp_err_t SPI_Select();
esp_err_t SPI_Deselect();
esp_err_t SPI_Write(uint8_t *data, size_t length);
esp_err_t SPI_Read(uint8_t *data, size_t length);

#endif // SPI_H