// LSM6DSL_SPI.h
// Date Created: 2024-10-24
// Date Updated: 2024-10-24
// LSM SPI header file

#ifndef LSM6DSL_H_
#define LSM6DSL_H_

/************************************Includes***************************************/

#include <stdint.h>

/************************************Includes***************************************/

/*************************************Defines***************************************/

#define LSM6DSL_WHO_AM_I_REG    0x0F
#define LSM6DSL_WHOAMI          0x6A
// LSM6DSL register addresses for accelerometer
#define ACCEL_X_L           0x28
#define ACCEL_X_H           0x29
#define ACCEL_Y_L           0x2A
#define ACCEL_Y_H           0x2B
#define ACCEL_Z_L           0x2C
#define ACCEL_Z_H           0x2D

/*************************************Defines***************************************/

void LSM6DSL_Init();
uint8_t LSM6DSL_Read_WHOAMI(void); 
int16_t read_accel_axis(uint8_t reg_low, uint8_t reg_high);

#endif // SLM6DSL_H