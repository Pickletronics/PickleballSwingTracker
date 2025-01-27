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

#define MAX_PROCESSING_THREADS      3
#define MAX_SPIFFS_THREADS          2*MAX_PROCESSING_THREADS

#define FSM_TAG "FSM"
#define PROCESSING_TAG "PROCESSING"
#define SPIFFS_WRITE_TAG "SPIFFS_WRITE"

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

/***********************************Externs*****************************************/

/****************************Data Structure Definitions*****************************/

typedef struct vector3D_t {
    float x, y, z;
} vector3D_t;

typedef struct data_processing_packet_t {
    char* SPIFFS_file_path;
    bool active;
    uint32_t num_samples;
    uint32_t impact_start_index;
    IMU_sample_t* processing_buffer;
} data_processing_packet_t;

typedef struct SPIFFS_packet_t {
    char* SPIFFS_file_path;
    bool active;
    uint16_t test_1;
    uint16_t test_2;
    uint16_t test_3;
} SPIFFS_packet_t;

enum BUTTON_ACTION {
    HOLD = -1,
    SINGLE_PRESS = 1,
    DOUBLE_PRESS = 2,
    NUM_ACTIONS
};

enum STATE {
    START,
    RESET,
    PLAY_SESSION,
    BLE_SESSION,
    NUM_STATES
};

typedef struct TaskHandle_Packet_t {
    TaskHandle_t Play_Session_Handle;
    TaskHandle_t Data_Processing_Handles[MAX_PROCESSING_THREADS];
    TaskHandle_t SPIFFS_Write_Handles[MAX_SPIFFS_THREADS];
} TaskHandle_Packet_t;

typedef struct state_t {
    enum STATE current_state;
    enum STATE next_state;
    bool skip_button_input;
    bool play_session_active;
    bool BLE_session_active;
    TaskHandle_Packet_t task_handles;
} state_t;

/****************************Data Structure Definitions*****************************/

/********************************Public Functions***********************************/

void Play_Session_task(void *args); 
void Button_task(void *args);
void Process_Data_task(void *args); 
void FSM_task(void *args);
void SPIFFS_Test_task(void *args);
void SPIFFS_Write_task(void *args);

/********************************Public Functions***********************************/

/****************************Interrupt Service Routines*****************************/

void Button_ISR();

/****************************Interrupt Service Routines*****************************/

#endif // THREADS_H