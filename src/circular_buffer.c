// circular_buffer.c

/************************************Includes***************************************/

#include "circular_buffer.h"

/************************************Includes***************************************/

/********************************Public Variables***********************************/

static circular_buffer_t buffers[MAX_NUM_BUFFERS] = {0};

/********************************Public Variables***********************************/

/********************************Public Functions***********************************/

bool Circular_Buffer_Init(uint32_t buffer_index) {
    if (buffer_index >= MAX_NUM_BUFFERS) {
        printf("Invalid buffer index\n");
        return false;
    } else {
        buffers[buffer_index].index = &(buffers[buffer_index].buffer[0]);
        return true;
    }
}

void Circular_Buffer_Write(uint32_t buffer_index, IMU_sample_t data) {
    // Validate buffer index
    if (buffer_index >= MAX_NUM_BUFFERS) {
        printf("Invalid buffer index\n");
        return;
    }

    // write data
    *buffers[buffer_index].index = data;

    // advance pointer
    Circular_Buffer_Increment_Index(buffer_index);
}

void Circular_Buffer_Peek(uint32_t buffer_index, IMU_sample_t* data) {
    // Validate buffer index
    if (buffer_index >= MAX_NUM_BUFFERS) {
        printf("Invalid buffer index\n");
        return;
    }

    // start at current index
    IMU_sample_t* peek_ptr = buffers[buffer_index].index;

    // move peek pointer to previous index
    Circular_Buffer_Decrement_Temp_Index(buffer_index, peek_ptr);

    // populate data pointer with data found at peek pointer
    *data = *peek_ptr;
}

IMU_sample_t* Circular_Buffer_Sized_DDump(uint32_t buffer_index, uint32_t num_samples_requested) {
    // Validate buffer index
    if (buffer_index >= MAX_NUM_BUFFERS) {
        printf("Invalid buffer index\n");
        return NULL;
    }

    IMU_sample_t* dump_buffer = malloc(num_samples_requested * sizeof(IMU_sample_t));

    // Validate dump buffer has been allocated
    if (dump_buffer == NULL) {
        printf("Memory allocation failed\n");
        return NULL;
    }

    // decrement pointer by num_samples_requested starting at current index
    IMU_sample_t* dump_ptr = buffers[buffer_index].index;
    for (uint32_t i = 0; i < num_samples_requested; i++) {
        Circular_Buffer_Decrement_Temp_Index(buffer_index, &dump_ptr);
    }

    // add num_samples_requested number of samples to dump buffer
    for (uint32_t i = 0; i < num_samples_requested; i++) {
        // printf("dump_ptr address: %p\n", (void*)dump_ptr);
        dump_buffer[i] = *dump_ptr;
        Circular_Buffer_Increment_Temp_Index(buffer_index, &dump_ptr);
    }

    return dump_buffer;
}

/********************************Public Functions***********************************/

/********************************Private Functions***********************************/

void Circular_Buffer_Increment_Index(uint32_t buffer_index) {
    if (buffers[buffer_index].index >= &(buffers[buffer_index].buffer[BUFFER_SIZE-1])) {\
        // reset pointer back to front of array
        buffers[buffer_index].index = &(buffers[buffer_index].buffer[0]);
    }
    else {
        // increment pointer normally
        buffers[buffer_index].index++;
    }
}

void Circular_Buffer_Increment_Temp_Index(uint32_t buffer_index, IMU_sample_t** temp_ptr) {
    if (*temp_ptr >= &(buffers[buffer_index].buffer[BUFFER_SIZE-1])) {\
        // reset pointer back to front of array
        *temp_ptr = &(buffers[buffer_index].buffer[0]);
    }
    else {
        // increment pointer normally
        (*temp_ptr)++;
    }
}

void Circular_Buffer_Decrement_Temp_Index(uint32_t buffer_index, IMU_sample_t** temp_ptr) {
    if (*temp_ptr == &(buffers[buffer_index].buffer[0])) {\
        // grab last index if at front of array
        *temp_ptr = &(buffers[buffer_index].buffer[BUFFER_SIZE-1]);
    }
    else {
        // decrement pointer normally
        (*temp_ptr)--;
    }
}

/********************************Private Functions***********************************/