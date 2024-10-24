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

/*************************************Defines***************************************/

void LSM6DSL_Init();
uint8_t LSM6DSL_Read_WHOAMI(void); 

#endif // SLM6DSL_H