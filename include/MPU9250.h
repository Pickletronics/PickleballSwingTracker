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

#define IMU_TAG "IMU"

#define MPU9250_CS              GPIO_NUM_5
#define MPU9250_MAX_CLK         1000000

#define MPU9250_WHOAMI          117
#define MPU_9250_PWR_MGMT_1     107

#define SENSITIVITY             (16.0f * 9.81f / 32768.0f)

#define ACCEL_CFG_1             28
#define ACCEL_CFG_2             29
#define ACCEL_XH                59
#define ACCEL_XL                60
#define ACCEL_YH                61
#define ACCEL_YL                62
#define ACCEL_ZH                63
#define ACCEL_ZL                64

#define GYRO_CFG                27
#define GYRO_XH                 67
#define GYRO_XL                 68
#define GYRO_YH                 69
#define GYRO_YL                 70
#define GYRO_ZH                 71
#define GYRO_ZL                 72

#define USER_CTRL               0x6A
#define I2C_MST_EN              0x20
#define I2C_MST_CTRL            0x24
#define I2C_SLV0_ADDR           0x25
#define I2C_SLV0_REG            0x26
#define I2C_SLV0_CTRL           0x27
#define I2C_SLV0_EN             0x80

#define I2C_SLV0_DO             0x63
#define EXT_SENS_DATA_00        0x49

#define AK8963_ADDR             0x0C

#define MAG_WHOAMI              0x00

#define MAG_CFG               0x0A
#define AK8963_ST1            0x02
#define AK8963_ST2            0x09

#define MAG_XL                0x03
#define MAG_XH                0x04
#define MAG_YL                0x05
#define MAG_YH                0x06
#define MAG_ZL                0x07
#define MAG_ZH                0x08

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

bool MPU9250_Init(void);
void MPU9250_update();

// PLOT FUNC SERIAL MONITOR SETTINGS:
    // - 3 channels, Simple Binary, int16, Little Endian
void MPU9250_plot_accel();
void MPU9250_plot_accel_mag();
void MPU9250_plot_gyro();

/********************************Public Functions***********************************/

#endif // MPU9250_H