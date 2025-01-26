// threads.h

#ifndef THREADS_H_
#define THREADS_H_

/************************************Includes***************************************/

#include "UART.h"
#include "SPI.h"
#include "MPU9250.h"
#include "button.h"
#include "BLE.h"
#include "SPIFFS.h"
#include "circular_buffer.h"

/************************************Includes***************************************/

/*************************************Defines***************************************/

#define MAX_NUM_PACKETS      3

/*************************************Defines***************************************/

/***********************************Externs*****************************************/

extern MPU9250_handle_t mpu;
extern SemaphoreHandle_t SPI_sem;
extern SemaphoreHandle_t UART_sem;
extern SemaphoreHandle_t Button_sem;
extern SemaphoreHandle_t SPIFFS_sem;
extern gptimer_handle_t Button_timer;
extern QueueHandle_t Button_queue;
extern QueueHandle_t Sample_queue; 

// FIXME: Would rather include function's header file
extern int esp_clk_cpu_freq();

/***********************************Externs*****************************************/

/****************************Data Structure Definitions*****************************/

typedef struct data_processing_packet_t {
    bool active;
    uint32_t packet_num;
    uint32_t num_samples;
    uint32_t impact_start_index;
    IMU_sample_t* processing_buffer;
} data_processing_packet_t;

typedef struct vector3D_t {
    float x, y, z;
} vector3D_t;

typedef struct SPIFFS_test_t {
    uint16_t blah;
    uint16_t blah2;
    uint16_t blah3;
} SPIFFS_test_t;

/****************************Data Structure Definitions*****************************/

/********************************Public Functions***********************************/

void Sample_Sensor_task(void *args); 
void Button_task(void *args);
void Process_Data_task(void *args); 
void Timer_task(void *args);
void SPIFFS_Test_task(void *args);
void SPIFFS_Write_task(void *args);

/********************************Public Functions***********************************/

/****************************Interrupt Service Routines*****************************/

void Button_ISR();

/****************************Interrupt Service Routines*****************************/

#endif // THREADS_H