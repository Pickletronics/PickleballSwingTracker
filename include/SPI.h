// SPI.h

#ifndef SPI_H_
#define SPI_H_

/************************************Includes***************************************/

#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"

/************************************Includes***************************************/

/*************************************Defines***************************************/

#define SPI_NUM             SPI2_HOST
#define PIN_NUM_MISO        GPIO_NUM_20
#define PIN_NUM_MOSI        GPIO_NUM_18
#define PIN_NUM_CLK         GPIO_NUM_19

/*************************************Defines***************************************/

/***********************************Externs*****************************************/
/***********************************Externs*****************************************/

/********************************Public Functions***********************************/

void SPI_Init();
esp_err_t SPI_Select(uint8_t CS_num);
esp_err_t SPI_Deselect(uint8_t CS_num);
esp_err_t SPI_Write(uint8_t *data, size_t length);
esp_err_t SPI_Read(uint8_t *data, size_t length);
esp_err_t SPI_Write_Read(uint8_t reg, uint8_t *data, size_t length);


/********************************Public Functions***********************************/

#endif // SPI_H