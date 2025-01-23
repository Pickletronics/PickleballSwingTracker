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
/*************************************Defines***************************************/

/***********************************Externs*****************************************/

extern MPU9250_handle_t mpu;
extern SemaphoreHandle_t SPI_sem;
extern SemaphoreHandle_t Button_sem;
extern gptimer_handle_t Button_timer;
extern QueueHandle_t Button_queue;
extern QueueHandle_t Sample_queue; 

// FIXME: Would rather include function's header file
extern int esp_clk_cpu_freq();

/***********************************Externs*****************************************/

/****************************Data Structure Definitions*****************************/

typedef struct Vector3D {
    float x, y, z;
} Vector3D;

typedef struct SPIFFS_test_t {
    uint16_t blah;
    uint16_t blah2;
    uint16_t blah3;
} SPIFFS_test_t;

/****************************Data Structure Definitions*****************************/

/********************************Public Functions***********************************/

void SEM_test(void *args);
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