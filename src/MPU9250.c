// MPU9250_SPI.c

/************************************Includes***************************************/

#include "MPU9250.h"

/************************************Includes***************************************/

/********************************Public Variables***********************************/

// Global handle for MPU9250
MPU9250_handle_t mpu;

/********************************Public Variables***********************************/

/********************************Public Functions***********************************/

void MPU9250_Init() {
    // configure MPU9250 as SPI device
    spi_device_interface_config_t MPU9250cfg = {
        .clock_speed_hz = MPU9250_MAX_CLK,
        .mode = 3,
        .spics_io_num = MPU9250_CS,
        .queue_size = 7,
    };

    esp_err_t ret = spi_bus_add_device(SPI_NUM, &MPU9250cfg, &spi_handle);
    if (ret != ESP_OK) {
        printf("Failed to add device to SPI bus: %s\n", esp_err_to_name(ret));
    }

    // Configure Chip Select
    gpio_set_direction(MPU9250_CS, GPIO_MODE_OUTPUT);
    SPI_Deselect(MPU9250_CS);
    
    uint8_t temp = 0;

    // wake up chip
    MPU9250_write(MPU_9250_PWR_MGMT_1, 0x00);

    /* initialize accel */

    // sensitivity - cfg 1
    temp = MPU9250_read(ACCEL_CFG_1);
    temp |= 0x08; // +/- 4g mode
    MPU9250_write(ACCEL_CFG_1, temp);
    temp = 0;

    // filtering - cfg 2

    /* initialize gyro */
    temp = MPU9250_read(GYRO_CFG);
    temp |= 0x08; // +/- 500 dps
    MPU9250_write(GYRO_CFG, temp);
    temp = 0;

    /* initialize mag */
}

void MPU9250_write(uint8_t reg, uint8_t val) {
    uint8_t buf[2];
    buf[0] = reg &= 0x7F;
    buf[1] = val;
    SPI_Select(MPU9250_CS); //turn on CS
    SPI_Write(buf, 2); // send reg address with write bit on and then data
    SPI_Deselect(MPU9250_CS); //turn off CS
}

uint8_t MPU9250_read(uint8_t reg) {
    uint8_t data = 0;
    reg |= 0x80;
    
    SPI_Select(MPU9250_CS); //turn on CS
    SPI_Write(&reg, 1); // send reg address with read bit on 
    SPI_Read(&data, 1); //read response from MPU9250
    SPI_Deselect(MPU9250_CS); //turn off CS

    return data;
}

uint8_t MPU9250_read_WHOAMI() {
    return MPU9250_read(MPU9250_WHOAMI);  
}

void MPU9250_update() {
    // read acccel x, y, z
    mpu.accel.x = (MPU9250_read(ACCEL_XH) << 8) | MPU9250_read(ACCEL_XL);
    mpu.accel.y = (MPU9250_read(ACCEL_YH) << 8) | MPU9250_read(ACCEL_YL);
    mpu.accel.z = (MPU9250_read(ACCEL_ZH) << 8) | MPU9250_read(ACCEL_ZL);

    // read gyro x, y, z
    mpu.gyro.x = (MPU9250_read(GYRO_XH) << 8) | MPU9250_read(GYRO_XL);
    mpu.gyro.y = (MPU9250_read(GYRO_YH) << 8) | MPU9250_read(GYRO_YL);
    mpu.gyro.z = (MPU9250_read(GYRO_ZH) << 8) | MPU9250_read(GYRO_ZL);
}

/********************************Public Functions***********************************/

/**********************************Test Threads*************************************/

void MPU9250_plot_accel() {
    while(1) {
        MPU9250_update();

        // output data over uart
        char accel[6] = {
            (mpu.accel.x >> 8) & 0xFF,
            (mpu.accel.x) & 0xFF,
            (mpu.accel.y >> 8) & 0xFF,
            (mpu.accel.y) & 0xFF,
            (mpu.accel.z >> 8) & 0xFF,
            (mpu.accel.z) & 0xFF,
        };

        UART_write(accel, 6);
    }
}

void MPU9250_plot_accel_mag() {
    while(1) {
        MPU9250_update();

        // output data over uart
        uint16_t mag_accel = sqrtf(mpu.accel.x * mpu.accel.x + mpu.accel.y * mpu.accel.y + mpu.accel.z * mpu.accel.z);
        char accel[2] = {
            (mag_accel >> 8) & 0xFF,
            (mag_accel) & 0xFF,
        };

        UART_write(accel, 2);
    }
}

void MPU9250_plot_gyro() {
    while(1) {
        MPU9250_update();

        // output data over uart
        char gyro[6] = {
            (mpu.gyro.x >> 8) & 0xFF,
            (mpu.gyro.x) & 0xFF,
            (mpu.gyro.y >> 8) & 0xFF,
            (mpu.gyro.y) & 0xFF,
            (mpu.gyro.z >> 8) & 0xFF,
            (mpu.gyro.z) & 0xFF,
        };

        UART_write(gyro, 6);
    }
}

/**********************************Test Threads*************************************/