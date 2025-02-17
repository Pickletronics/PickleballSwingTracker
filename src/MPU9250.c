// MPU9250_SPI.c

/************************************Includes***************************************/

#include "MPU9250.h"

/************************************Includes***************************************/

/********************************Public Variables***********************************/

// Global handle for MPU9250
MPU9250_handle_t mpu;

bool init_mag = false;

/********************************Public Variables***********************************/

/*********************************Static Functions**********************************/

static void MPU9250_write(uint8_t reg, uint8_t val) {
    uint8_t buf[2];
    buf[0] = reg &= 0x7F;
    buf[1] = val;
    SPI_Select(MPU9250_CS); //turn on CS
    SPI_Write(buf, 2); // send reg address with write bit on and then data
    SPI_Deselect(MPU9250_CS); //turn off CS
}

static uint8_t MPU9250_read(uint8_t reg) {
    uint8_t data = 0;
    reg |= 0x80;
    
    SPI_Select(MPU9250_CS); //turn on CS
    SPI_Write(&reg, 1); // send reg address with read bit on 
    SPI_Read(&data, 1); //read response from MPU9250
    SPI_Deselect(MPU9250_CS); //turn off CS

    return data;
}

static uint8_t MPU9250_read_WHOAMI() {
    return MPU9250_read(MPU9250_WHOAMI);  
}

static void MPU9250_read_accelerometer() {
    uint8_t data[6];

    SPI_Select(MPU9250_CS); // turn on CS
    SPI_Write_Read(ACCEL_XH, data, 6); // start from XH, end on ZL
    SPI_Deselect(MPU9250_CS); // turn off CS

    mpu.accel.x = (int16_t)((data[0] << 8) | data[1]);
    mpu.accel.y = (int16_t)((data[2] << 8) | data[3]);
    mpu.accel.z = (int16_t)((data[4] << 8) | data[5]);
}

static void MPU9250_read_gyroscope() {
    uint8_t data[6];

    SPI_Select(MPU9250_CS); // turn on CS
    SPI_Write_Read(GYRO_XH, data, 6); // start from XH, end on ZL
    SPI_Deselect(MPU9250_CS); // turn off CS

    mpu.gyro.x = (int16_t)((data[0] << 8) | data[1]);
    mpu.gyro.y = (int16_t)((data[2] << 8) | data[3]);
    mpu.gyro.z = (int16_t)((data[4] << 8) | data[5]);
}

static uint8_t AK8963_read(uint8_t reg) {
    MPU9250_write(I2C_SLV0_ADDR, 0x80 | AK8963_ADDR);  // set address (read)
    MPU9250_write(I2C_SLV0_REG, reg);  // set reg-to-read address
    MPU9250_write(I2C_SLV0_CTRL, I2C_SLV0_EN | 0x01);  // enable 1 byte read
    vTaskDelay(pdTICKS_TO_MS(1));
    return MPU9250_read(EXT_SENS_DATA_00);
}

static void AK8963_write(uint8_t reg, uint8_t data) {
    MPU9250_write(I2C_SLV0_ADDR, AK8963_ADDR); // Set address (write)
    MPU9250_write(I2C_SLV0_REG, reg); // Set reg-to-write address
    MPU9250_write(I2C_SLV0_DO, data); // Set data to write
    MPU9250_write(I2C_SLV0_CTRL, I2C_SLV0_EN | 0x01); // enable 1 byte write
    vTaskDelay(pdTICKS_TO_MS(1));
}

static void MPU9250_read_magnetometer() {
    // only sample when data is ready
    if (AK8963_read(AK8963_ST1) & 0x01) {
        MPU9250_write(I2C_SLV0_ADDR, 0x80 | AK8963_ADDR);  // set address (read)
        MPU9250_write(I2C_SLV0_REG, MAG_XL);  // start at XL
        MPU9250_write(I2C_SLV0_CTRL, I2C_SLV0_EN | 0x06);  // enable 6 byte read -> XL,XH,YL,YH,ZL,ZH
        vTaskDelay(pdTICKS_TO_MS(1));

        uint8_t data[6];

        SPI_Select(MPU9250_CS); // turn on CS
        SPI_Write_Read(EXT_SENS_DATA_00, data, 6); // start at EXT_SENS_DATA_00
        SPI_Deselect(MPU9250_CS); // turn off CS

        mpu.mag.x = (int16_t)((data[1] << 8) | data[0]);
        mpu.mag.y = (int16_t)((data[3] << 8) | data[2]);
        mpu.mag.z = (int16_t)((data[5] << 8) | data[4]);

        // printf("x: %d\n", mpu.mag.x);
        // printf("y: %d\n", mpu.mag.y);
        // printf("z: %d\n\n", mpu.mag.z);

        AK8963_read(AK8963_ST2); // read to finish
    }
}

/*********************************Static Functions**********************************/

/********************************Public Functions***********************************/

bool MPU9250_Init() {
    // configure MPU9250 as SPI device
    spi_device_interface_config_t MPU9250cfg = {
        .clock_speed_hz = MPU9250_MAX_CLK,
        .mode = 3,
        .spics_io_num = MPU9250_CS,
        .queue_size = 7,
    };

    esp_err_t ret = spi_bus_add_device(SPI_NUM, &MPU9250cfg, &spi_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(IMU_TAG,"Failed to add device to SPI bus: %s\n", esp_err_to_name(ret));
        return false;
    }

    // Configure Chip Select
    gpio_set_direction(MPU9250_CS, GPIO_MODE_OUTPUT);
    SPI_Deselect(MPU9250_CS);
    
    uint8_t temp = 0;

    // wake up chip
    MPU9250_write(PWR_MGMT_1, 0x00);

    // who am i?
    temp = MPU9250_read_WHOAMI();
    if (temp == 0x71) {
        ESP_LOGI(IMU_TAG, "MPU9250 dectected.");
        init_mag = true;
    }
    else {

        if (temp == 0x70) {
            ESP_LOGE(IMU_TAG, "MPU6500 dectected; No magnetometer.");
        }
        else {
            ESP_LOGE(IMU_TAG, "Incorrect IMU dectected.");
            return false;
        }
        
    }

    /* initialize accel */

    // sensitivity - cfg 1
    temp = MPU9250_read(ACCEL_CFG_1);
    temp |= 0x0C; // +/- 16g mode
    MPU9250_write(ACCEL_CFG_1, temp);
    temp = 0;

    // filtering - cfg 2

    /* initialize gyro */
    temp = MPU9250_read(GYRO_CFG);
    temp |= 0x08; // +/- 500 dps
    MPU9250_write(GYRO_CFG, temp);
    temp = 0;

    /* initialize mag */
    if (init_mag) {
        // init MPU9250 as I2C master to talk to mag
        MPU9250_write(USER_CTRL, I2C_MST_EN);
        MPU9250_write(I2C_MST_CTRL, 0x0D); // 400 kHz I2C clk

        // who am i?
        temp = AK8963_read(AK8963_WHOAMI);
        if (temp == 0x48) {
            ESP_LOGI(IMU_TAG, "AK8963 dectected.");
        }
        else {
            ESP_LOGE(IMU_TAG, "AK8963 not dectected.");
            return false;
        }

        // continuous conversions
        AK8963_write(AK8963_CFG, 0x00);  // power down
        vTaskDelay(pdTICKS_TO_MS(10));
        AK8963_write(AK8963_CFG, 0x16);  // continuous measurement mode 2 (100 Hz)
        vTaskDelay(pdTICKS_TO_MS(10));
    }

    return true;
}

void MPU9250_update() {
    MPU9250_read_accelerometer();
    MPU9250_read_gyroscope();
    if (init_mag) 
        MPU9250_read_magnetometer();
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