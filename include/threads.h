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

/************************************Includes***************************************/

/*************************************Defines***************************************/
/*************************************Defines***************************************/

/***********************************Externs*****************************************/

extern MPU9250_handle_t mpu;
extern SemaphoreHandle_t SPI_sem;
extern QueueHandle_t data_queue; 
extern SemaphoreHandle_t Button_sem;
// extern SemaphoreHandle_t dataDump_sem;

// FIXME: Would rather include function's header file
extern int esp_clk_cpu_freq();

/***********************************Externs*****************************************/

/********************************Public Functions***********************************/

void SEM_test(void *args);
void SPI_test(void *args); 
void Button_task(void *args);
void DumpData_task(void *args); 

/********************************Public Functions***********************************/

/****************************Interrupt Service Routines*****************************/

void Button_ISR();

/****************************Interrupt Service Routines*****************************/

#endif // THREADS_H