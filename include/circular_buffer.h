// circular_buffer.h

#ifndef CIRCULAR_BUFFER_H_
#define CIRCULAR_BUFFER_H_

/************************************Includes***************************************/

#include "MPU9250.h"

/************************************Includes***************************************/

/*************************************Defines***************************************/

#define BUFFER_SIZE         2048
#define MAX_NUM_BUFFERS     1

#define IMU_BUFFER          0

/*************************************Defines***************************************/

/****************************Data Structure Definitions*****************************/

typedef struct IMU_sample_t {
    TickType_t time;
    MPU9250_handle_t IMU;
} IMU_sample_t;

typedef struct circular_buffer_t {
    IMU_sample_t buffer[BUFFER_SIZE];
    IMU_sample_t* index;
} circular_buffer_t;

/****************************Data Structure Definitions*****************************/

/***********************************Externs*****************************************/
/***********************************Externs*****************************************/

/********************************Public Functions***********************************/

bool Circular_Buffer_Init(uint32_t buffer_index);
void Circular_Buffer_Write(uint32_t buffer_index, IMU_sample_t data);
void Circular_Buffer_Peek(uint32_t buffer_index, IMU_sample_t* data);
void Circular_Buffer_Get_Index(uint32_t buffer_index);
void Circular_Buffer_Print(uint32_t buffer_index);

// use dynamic allocation
IMU_sample_t* Circular_Buffer_Sized_DDump(uint32_t buffer_index, uint32_t num_samples_requested);
IMU_sample_t* Circular_Buffer_Ranged_DDump(uint32_t buffer_index, IMU_sample_t start, uint32_t num_samples_L, uint32_t num_samples_R);

/********************************Public Functions***********************************/

/********************************Private Functions***********************************/

void Circular_Buffer_Increment_Index(uint32_t buffer_index);
void Circular_Buffer_Increment_Temp_Index(uint32_t buffer_index, IMU_sample_t** temp_ptr);
void Circular_Buffer_Decrement_Temp_Index(uint32_t buffer_index, IMU_sample_t** temp_ptr);

/********************************Private Functions***********************************/

#endif // CIRCULAR_BUFFER_H