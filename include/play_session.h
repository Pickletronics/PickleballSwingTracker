// play_session.h

#ifndef PLAY_SESSION_H_
#define PLAY_SESSION_H_

/************************************Includes***************************************/

#include "UART.h"
#include "SPI.h"
#include "MPU9250.h"
#include "button.h"
#include "LED.h"
#include "BLE.h"
#include "SPIFFS.h"
#include "circular_buffer.h"
#include "esp_log.h"

/************************************Includes***************************************/

/*************************************Defines***************************************/

#define MAX_PROCESSING_THREADS      3
#define MAX_SPIFFS_THREADS          2*MAX_PROCESSING_THREADS
#define MAX_PLAY_SESSIONS           5

#define FSM_TAG "FSM"
#define PROCESSING_TAG "PROCESSING"
#define SPIFFS_WRITE_TAG "SPIFFS_WRITE"
#define PLAY_SESSION_TAG "PLAY_SESSION"

/*************************************Defines***************************************/

/***********************************Externs*****************************************/

extern MPU9250_handle_t mpu;
extern SemaphoreHandle_t SPI_sem;
extern SemaphoreHandle_t UART_sem;
extern SemaphoreHandle_t Button_sem;
extern SemaphoreHandle_t SPIFFS_sem;
extern gptimer_handle_t Button_timer;
extern QueueHandle_t Button_queue;

// FIXME: Would rather include function's header file
extern int esp_clk_cpu_freq();

extern TaskHandle_t LED_Handle;

/***********************************Externs*****************************************/

/****************************Data Structure Definitions*****************************/

typedef struct vector3D_t {
    float x, y, z;
} vector3D_t;

typedef struct data_processing_packet_t {
    // FILE* f;
    char* SPIFFS_file_path;
    bool active;
    uint32_t num_samples;
    uint32_t impact_start_index;
    IMU_sample_t* processing_buffer;
} data_processing_packet_t;

typedef struct SPIFFS_packet_t {
    // FILE* f;
    char* SPIFFS_file_path;
    bool active;
    uint16_t test_1;
    uint16_t test_2;
    uint16_t test_3;
} SPIFFS_packet_t;

typedef struct SPIFFS_files_t {
    uint8_t num_files; 
    char* file_path[MAX_PLAY_SESSIONS];
} SPIFFS_files_t;
extern SPIFFS_files_t SPIFFS_files;

/****************************Data Structure Definitions*****************************/

/********************************Public Functions***********************************/

void Play_Session_task(void *args); 
void Process_Data_task(void *args); 
void SPIFFS_Write_task(void *args);

/********************************Public Functions***********************************/

/****************************Interrupt Service Routines*****************************/
/****************************Interrupt Service Routines*****************************/

#endif // PLAY_SESSION_H