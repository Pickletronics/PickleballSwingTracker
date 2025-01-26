// MPU9250.h

#ifndef MPU9250_H_
#define MPU9250_H_

/************************************Includes***************************************/

#include <stdint.h>
#include "SPI.h"
#include "UART.h"
#include <math.h>

/************************************Includes***************************************/

/*************************************Defines***************************************/

#define MPU9250_CS              GPIO_NUM_5
#define MPU9250_MAX_CLK         1000000

#define MPU9250_WHOAMI          117
#define MPU_9250_PWR_MGMT_1     107

#define SENSITIVITY             (4.0f * 9.81f / 32768.0f)

#define ACCEL_CFG_1             28
#define ACCEL_CFG_2             29
#define ACCEL_XH                59
#define ACCEL_XL                60
#define ACCEL_YH                61
#define ACCEL_YL                62
#define ACCEL_ZH                63
#define ACCEL_ZL                64

#define GYRO_CFG               27
#define GYRO_XH                67
#define GYRO_XL                68
#define GYRO_YH                69
#define GYRO_YL                70
#define GYRO_ZH                71
#define GYRO_ZL                72

/*************************************Defines***************************************/

/****************************Data Structure Definitions*****************************/

typedef struct accel_data_t {
    int16_t x, y, z;
} accel_data_t;

typedef struct gyro_data_t {
    int16_t x, y, z;
} gyro_data_t;

typedef struct mag_data_t {
    int16_t x, y, z;
} mag_data_t;

typedef struct MPU9250_handle_t {
    accel_data_t accel;
    gyro_data_t gyro;
    mag_data_t mag;
} MPU9250_handle_t;

/****************************Data Structure Definitions*****************************/

/***********************************Externs*****************************************/

extern spi_device_handle_t spi_handle;

/***********************************Externs*****************************************/

/********************************Public Functions***********************************/

void MPU9250_Init(void);
void MPU9250_write(uint8_t reg, uint8_t val);
uint8_t MPU9250_read(uint8_t reg);
uint8_t MPU9250_read_WHOAMI();
void MPU9250_read_accelerometer();
void MPU9250_read_gyroscope();
void MPU9250_update();

// PLOT FUNC SERIAL MONITOR SETTINGS:
    // - 3 channels, Simple Binary, int16, Little Endian
void MPU9250_plot_accel();
void MPU9250_plot_accel_mag();
void MPU9250_plot_gyro();

/********************************Public Functions***********************************/

#endif // MPU9250_H