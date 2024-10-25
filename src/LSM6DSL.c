// LSM6DSL_SPI.c
// Date Created: 2024-10-24
// Date Updated: 2024-10-24
// Defines for LSM SPI functions

/************************************Includes***************************************/

#include "LSM6DSL.h"
#include "SPI.h"

/********************************Public Functions***********************************/

void LSM6DSL_Init(){
     ESP_LOGI("LSM6DSL", "LSM6DSL initialized.");
}

uint8_t LSM6DSL_Read_WHOAMI() {
    uint8_t WHOAMI_reg = LSM6DSL_WHO_AM_I_REG | 0x80; //turn on MSB for reads 
    uint8_t WHOAMI_val = 0; 

    SPI_Select(); //turn on CS
    SPI_Write(&WHOAMI_reg, 1); //send reg address with read bit on 
    SPI_Read(&WHOAMI_val, 1); //read response from LSM6DSL
    SPI_Deselect(); //turn off CS

    //SPI_Transmit(&WHOAMI_reg, &WHOAMI_val, 1); 

    return WHOAMI_val;  
}