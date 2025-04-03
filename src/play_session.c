// play_session.c

/************************************Includes***************************************/

#include "play_session.h"

/************************************Includes***************************************/

/********************************Public Variables***********************************/

uint8_t num_sessions = 0; 

data_processing_packet_t play_session_packets[MAX_PROCESSING_THREADS];
SPIFFS_packet_t SPIFFS_packets[MAX_SPIFFS_THREADS];
SPIFFS_files_t SPIFFS_files = {
    .num_files = 0,
    .file_path = {NULL}
};
SPIFFS_write_data_t SPIFFS_data; 

/********************************Public Variables***********************************/

/********************************Public Functions***********************************/

// FILTERS
void moving_average_filter(float *data) {
    float sum = 0.0;
    int i;

    // Compute initial window sum
    for (i = 0; i < WINDOW_SIZE && i < NUM_SAMPLES_TOTAL; i++) {
        sum += data[i];
    }

    // Apply moving average filter in-place
    for (i = 0; i < NUM_SAMPLES_TOTAL - WINDOW_SIZE; i++) {
        data[i] = sum / WINDOW_SIZE;
        sum = sum - data[i] + data[i + WINDOW_SIZE];  // Slide window
    }

    // Handle the last few elements (optional: repeat last valid value)
    for (; i < NUM_SAMPLES_TOTAL; i++) {
        data[i] = sum / WINDOW_SIZE;
    }
}

// Sampling task
// Reads accelerometer and gyroscope data from IMU 
// and pushes to buffer for processing
void Play_Session_task(void *args) {
    // sampling settings
    IMU_sample_t sample;
    uint32_t sample_count = 0;

    // uint32_t num_samples = 0;
    // TickType_t init_time_sec = xTaskGetTickCount();
    // TickType_t curr_time_sec = xTaskGetTickCount();
    // TickType_t one_sec = pdMS_TO_TICKS(pdTICKS_TO_MS(1000));

    // circular buffer init
    Circular_Buffer_Init(IMU_BUFFER);

    // impact settings
    const float IMPACT_CHANGE_THRESHOLD = 77.0f;
    const TickType_t IMPACT_BUFFER_TIME = pdMS_TO_TICKS(50);
    TickType_t last_impact_time = 0;
    float prev_accel_magnitude = 0.0f;
    bool impact_detected = false;
    bool first_sample = true;

    // led init
    const gpio_num_t LED_PIN = 15;
    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_PIN, 1);

    // session SPIFFS init
    char file_name[32];     
    sprintf(file_name, "session_%u.txt", SPIFFS_files.num_files);
    char file_path[128]; 
    sprintf(file_path, "/spiffs/%s", file_name);

    if(SPIFFS_files.num_files < MAX_PLAY_SESSIONS){
        // Add new session to SPIFFS_files
        SPIFFS_files.file_path[SPIFFS_files.num_files] = strdup(file_path);

        // Add the header to the file // TODO: Add meaningful header
        char buffer[64];
        sprintf(buffer, "%u\n", SPIFFS_files.num_files);
        ESP_LOGI(PLAY_SESSION_TAG, "SPIFFS file created: %s", file_path);
        SPIFFS_Write(file_path, buffer);
        SPIFFS_files.num_files++;
    }
    else{
        ESP_LOGE(PLAY_SESSION_TAG, "MAX SESSIONS REACHED");
        LED_notify(SPIFFS_FULL);

        // do nothing
        while (1){
            // check for notification to delete
            uint32_t notification;
            if (xTaskNotifyWait(0, 0, &notification, 0) == pdTRUE) {
                vTaskDelete(NULL);
            }
        }
        
    }
    
    while(1){
        // check for notification to delete - clean exit
        uint32_t notification;
        if (xTaskNotifyWait(0, 0, &notification, 0) == pdTRUE) {
            vTaskDelete(NULL);
        }

        // populate sample
        MPU9250_update();
        sample.time = xTaskGetTickCount();
        sample.IMU = mpu;
        // write data
        Circular_Buffer_Write(IMU_BUFFER, sample);

        // num_samples++;
        // curr_time_sec = xTaskGetTickCount();
        // if (curr_time_sec > init_time_sec + one_sec) {
        //     printf("%ld samples per second\n", num_samples);
        //     num_samples = 0;
        //     init_time_sec = xTaskGetTickCount();
        // }

        // Convert raw accelerometer values to m/s^2
        vector3D_t accel_real = {
            sample.IMU.accel.x * ACCEL_SENSITIVITY,
            sample.IMU.accel.y * ACCEL_SENSITIVITY,
            sample.IMU.accel.z * ACCEL_SENSITIVITY
        };
        float accel_magnitude = sqrt(accel_real.x * accel_real.x + accel_real.y * accel_real.y + accel_real.z * accel_real.z);
        float magnitude_change = fabs(accel_magnitude - prev_accel_magnitude);

        if (first_sample) {
            // ignore first sample
            first_sample = false;
        }
        // Check for impact based on change in magnitude (ignore other "impacts" for IMPACT_BUFFER_TIME ms)
        else if ((magnitude_change > IMPACT_CHANGE_THRESHOLD) && (xTaskGetTickCount() - last_impact_time > IMPACT_BUFFER_TIME)) {
            // Impact detected
            last_impact_time = xTaskGetTickCount();
            impact_detected = true;
            // printf("Imapct at %ld\n", last_impact_time);
        }

        // Update the previous magnitude for the next iteration
        prev_accel_magnitude = accel_magnitude;

        // send impact to processing thread
        if (impact_detected) {
            // start counting samples to the right
            sample_count++;
            gpio_set_level(LED_PIN, 0);

            // once desired number of samples, dump buffer and spawn processing thread
            if (sample_count >= NUM_SAMPLES_IMPACT) {
                // find free packet
                int8_t packet_index = -1;
                for (int8_t i = 0; i < MAX_PROCESSING_THREADS; i++) {
                    if (!play_session_packets[i].active) {
                        packet_index = i;
                        break;
                    }
                }

                // FIXME: data is lost if no space left
                if (packet_index == -1) {
                    ESP_LOGE(PLAY_SESSION_TAG, "Max play session packets being processed");
                }
                else {
                    // populate data processing buffer
                    play_session_packets[packet_index].SPIFFS_file_path = file_path;
                    play_session_packets[packet_index].active = true;
                    play_session_packets[packet_index].num_samples = NUM_SAMPLES_TOTAL;
                    play_session_packets[packet_index].impact_start_index = NUM_SAMPLES_TOTAL - NUM_SAMPLES_IMPACT;
                    play_session_packets[packet_index].processing_buffer = Circular_Buffer_Sized_DDump(IMU_BUFFER, NUM_SAMPLES_TOTAL);
                    play_session_packets[packet_index].impact_time = xTaskGetTickCount(); 

                    // spawn data processing thread
                    // ESP_LOGI(PLAY_SESSION_TAG, "Impact detected - Spawning processing thread");
                    xTaskCreate(Process_Data_task, "Process_task", 8192, (void*)&play_session_packets[packet_index], 1, NULL);                 
                }

                // reset necessary variables
                sample_count = 0;
                impact_detected = false;
                gpio_set_level(LED_PIN, 1);
            }
        }
    }  
}

void Process_Data_task(void *args) {
    data_processing_packet_t* packet = (data_processing_packet_t*) args;
    float accel_magnitude[NUM_SAMPLES_TOTAL];
    float rotation_axis[NUM_SAMPLES_TOTAL];

    while (1) {

        SPIFFS_data.impact_time = pdTICKS_TO_MS(packet->impact_time);
        SPIFFS_data.impact_region = (float*)malloc(NUM_SAMPLES_IMPACT * sizeof(float));
        for (uint32_t i = 0; i < NUM_SAMPLES_TOTAL; i++) {

            // Convert raw accelerometer values to m/s^2
            vector3D_t accel_real = {
                packet->processing_buffer[i].IMU.accel.x * ACCEL_SENSITIVITY,
                packet->processing_buffer[i].IMU.accel.y * ACCEL_SENSITIVITY,
                packet->processing_buffer[i].IMU.accel.z * ACCEL_SENSITIVITY
            };

            accel_magnitude[i] = sqrt(accel_real.x * accel_real.x + accel_real.y * accel_real.y + accel_real.z * accel_real.z);
            
            // copy impact region for dump
            if (i >= packet->impact_start_index) {
                SPIFFS_data.impact_region[i-packet->impact_start_index] = accel_magnitude[i];
            }

            // get max accel before impact
            if (i < packet->impact_start_index) {
                // update max impact strength
                if (accel_magnitude[i] > SPIFFS_data.impact_strength) {
                    SPIFFS_data.impact_strength = accel_magnitude[i];
                }
            }

            // convert raw gyroscope data to dps
            vector3D_t gyro_real = {
                packet->processing_buffer[i].IMU.gyro.x * GYRO_SENSITIVITY,
                packet->processing_buffer[i].IMU.gyro.y * GYRO_SENSITIVITY,
                packet->processing_buffer[i].IMU.gyro.z * GYRO_SENSITIVITY
            };

            rotation_axis[i] = fabs(gyro_real.z);
            
        }

        // Filters
        moving_average(rotation_axis, NUM_SAMPLES_TOTAL); // MAV to remove impact from gyro
        SPIFFS_data.max_rotation = 0;
        SPIFFS_data.impact_rotation = rotation_axis[packet->impact_start_index];
        for (int i = 0; i < NUM_SAMPLES_TOTAL; i++) {
            // get max accel before impact
            if (i < packet->impact_start_index) {
                // update max gyro axis
                if (rotation_axis[i] > SPIFFS_data.max_rotation) {
                    SPIFFS_data.max_rotation = rotation_axis[i];
                }
            }
        }
        

        // // UART dump to serial plot
        // if (xSemaphoreTake(UART_sem, portMAX_DELAY) == pdTRUE) {   
        //      union {
        //         float real_data;
        //         uint32_t real_data_u32;
        //     } float_union; 

        //     printf("@");
        //     for (uint32_t i = 0; i < NUM_SAMPLES_TOTAL; i++) {

        //         // float_union.real_data = accel_magnitude[i];
        //         float_union.real_data = rotation_axis[i];

        //         // output data over uart
        //         char byte_data[4] = {
        //             (float_union.real_data_u32 >> 0) & 0xFF,
        //             (float_union.real_data_u32 >> 8) & 0xFF,
        //             (float_union.real_data_u32 >> 16) & 0xFF,
        //             (float_union.real_data_u32 >> 24) & 0xFF
        //         };

        //         // UART_write(byte_data, 4);
        //         printf("%.2f,",float_union.real_data);
        //     }
        //     printf("#");

        //     // Give up the semaphore 
        //     xSemaphoreGive(UART_sem);
        // }

        // printf("Impact strength: %.1f\n", SPIFFS_data.impact_strength);
        // printf("Impact rotation: %.1f\n", SPIFFS_data.impact_rotation);
        // printf("Max rotation: %.1f\n", SPIFFS_data.max_rotation);

        // find free packet
        int8_t packet_index = -1;
        for (int8_t i = 0; i < MAX_SPIFFS_THREADS; i++) {
            if (!SPIFFS_packets[i].active) {
                packet_index = i;
                break;
            }
        }

        // FIXME: data is lost if no space left
        if (packet_index == -1) {
            printf("Max SPIFFS packets being processed.\n");
        }
        else {
                // populate data processing buffer
                SPIFFS_packets[packet_index].SPIFFS_file_path = packet->SPIFFS_file_path;
                SPIFFS_packets[packet_index].active = true;
                SPIFFS_packets[packet_index].data = SPIFFS_data; 

                // spawn data processing thread
                xTaskCreate(SPIFFS_Write_task, "SPIFFS_Write_task", 8192, (void *)&SPIFFS_packets[packet_index], 3, NULL);
            

        }

        free(packet->processing_buffer);
        packet->active = false;

        vTaskDelete(NULL);
    }
    
}

void SPIFFS_Write_task(void *args){
    SPIFFS_packet_t *packet = (SPIFFS_packet_t *)args; 

    while(1){
        if (xSemaphoreTake(SPIFFS_sem, portMAX_DELAY) == pdTRUE) {
            size_t buffer_size = 512; 
            char buffer[buffer_size]; 
            size_t offset = 0; 

            // Write the accleration magnitude array to the file 
            SPIFFS_Write(packet->SPIFFS_file_path, "New Impact Data:\n"); 
            SPIFFS_Write(packet->SPIFFS_file_path, "Impact Array:\n"); 
            for(uint32_t i = 0; i < NUM_SAMPLES_IMPACT; i++){
                int written; 
                if(i == 0){
                    written = snprintf(buffer + offset, buffer_size - offset, "[%.1f,", packet->data.impact_region[i]);
                }
                else if(i == NUM_SAMPLES_IMPACT-1){
                    written = snprintf(buffer + offset, buffer_size - offset, "%.1f]\n", packet->data.impact_region[i]);
                }
                else {
                    written = snprintf(buffer + offset, buffer_size - offset, "%.1f,", packet->data.impact_region[i]);
                }

                if (written < 0 || offset + written >= buffer_size) {
                    // Buffer full, flush to SPIFFS
                    SPIFFS_Write(packet->SPIFFS_file_path, buffer);
                    offset = 0;
                    i--;  // Retry writing the same value after flushing
                    continue;
                }

                // Update the offset tracker
                offset += written; 
            }

            // Write any remaining data in the buffer
            if (offset > 0) {
                SPIFFS_Write(packet->SPIFFS_file_path, buffer);
            }

            // Write the impact strength and rotation to the file 
            SPIFFS_Write(packet->SPIFFS_file_path, "Impact Strength:\n"); 
            sprintf(buffer, "%.1f\n", packet->data.impact_strength); 
            SPIFFS_Write(packet->SPIFFS_file_path, buffer);

            SPIFFS_Write(packet->SPIFFS_file_path, "Impact Rotation:\n"); 
            sprintf(buffer, "%.1f\n", packet->data.impact_rotation); 
            SPIFFS_Write(packet->SPIFFS_file_path, buffer);

            SPIFFS_Write(packet->SPIFFS_file_path, "Max Rotation:\n"); 
            sprintf(buffer, "%.1f\n", packet->data.max_rotation); 
            SPIFFS_Write(packet->SPIFFS_file_path, buffer);

            SPIFFS_Write(packet->SPIFFS_file_path, "Time:\n"); 
            sprintf(buffer, "%ld\n", packet->data.impact_time); 
            SPIFFS_Write(packet->SPIFFS_file_path, buffer);

            // SPIFFS_Print(packet->SPIFFS_file_path); // debugging

            // Free dynamic memory created for accel array 
            free(SPIFFS_data.impact_region);
            SPIFFS_data.impact_region = NULL;
            packet->data.impact_region = NULL; 
            
            // Give up the semaphore 
            packet->active = false;
            xSemaphoreGive(SPIFFS_sem);

            vTaskDelete(NULL);
        }
    }
}

/********************************Public Functions***********************************/