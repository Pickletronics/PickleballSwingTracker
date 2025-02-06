// threads.c

/************************************Includes***************************************/

#include "threads.h"

/************************************Includes***************************************/

/********************************Public Variables***********************************/

volatile bool hold_detected = false;
volatile int8_t num_presses = 0;
uint8_t num_sessions = 0; 

// for testing purposes
bool impact_detected = false;

data_processing_packet_t play_session_packets[MAX_PROCESSING_THREADS];
SPIFFS_packet_t SPIFFS_packets[MAX_SPIFFS_THREADS];
SPIFFS_files_t SPIFFS_files = {
    .num_files = 0,
    .file_path = {NULL}
};

/********************************Public Variables***********************************/

/********************************Public Functions***********************************/

// Sampling task
// Reads accelerometer and gyroscope data from IMU 
// and pushes to buffer for processing
void Play_Session_task(void *args) {
    // sampling settings
    IMU_sample_t sample;
    uint32_t sample_count = 0;
    const uint32_t NUM_SAMPLES_WAIT = 400;
    const uint32_t NUM_SAMPLES_TOTAL = NUM_SAMPLES_WAIT*2;

    // circular buffer init
    Circular_Buffer_Init(IMU_BUFFER);

    // impact settings
    const float IMPACT_CHANGE_THRESHOLD = 20.0f;
    const TickType_t IMPACT_BUFFER_TIME = pdMS_TO_TICKS(50);
    float prev_accel_magnitude = 0.0f;
    bool impact_detected = false;
    bool first_sample = true;

    // demo led init
    const TickType_t LED_on_time = pdMS_TO_TICKS(500);
    TickType_t last_impact_time = 0;
    const gpio_num_t LED_PIN = 2;
    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

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

        // Convert raw accelerometer values to m/s^2
        vector3D_t accel_real = {
            sample.IMU.accel.x * SENSITIVITY,
            sample.IMU.accel.y * SENSITIVITY,
            sample.IMU.accel.z * SENSITIVITY
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

        // FIXME: for demo, turn on briefly
        if (xTaskGetTickCount() - last_impact_time < LED_on_time) {
            gpio_set_level(LED_PIN, 1);
        }
        else {
            gpio_set_level(LED_PIN, 0);
        }

        // send impact to processing thread
        if (impact_detected) {
            // start counting samples to the right
            sample_count++;

            // once desired number of samples, dump buffer and spawn processing thread
            if (sample_count >= NUM_SAMPLES_WAIT) {
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
                    play_session_packets[packet_index].impact_start_index = NUM_SAMPLES_TOTAL - NUM_SAMPLES_WAIT;
                    play_session_packets[packet_index].processing_buffer = Circular_Buffer_Sized_DDump(IMU_BUFFER, NUM_SAMPLES_TOTAL);

                    // spawn data processing thread
                    // ESP_LOGI(PLAY_SESSION_TAG, "Impact detected - Spawning processing thread");
                    xTaskCreatePinnedToCore(Process_Data_task, "Process_task", 4096, (void*)&play_session_packets[packet_index], 1, NULL, 1);                 
                }

                // reset necessary variables
                sample_count = 0;
                impact_detected = false;
            }
        }
    }  
}

void Process_Data_task(void *args) {
    data_processing_packet_t* packet = (data_processing_packet_t*) args;

    while (1) {

        // acquire UART sem
        // if (xSemaphoreTake(UART_sem, portMAX_DELAY) == pdTRUE) {    
        //     for (uint32_t i = 0; i < packet->num_samples; i++) {

        //         // Convert raw accelerometer values to m/s^2
        //         vector3D_t accel_real = {
        //             packet->processing_buffer[i].IMU.accel.x * SENSITIVITY,
        //             packet->processing_buffer[i].IMU.accel.y * SENSITIVITY,
        //             packet->processing_buffer[i].IMU.accel.z * SENSITIVITY
        //         };

        //         union {
        //             float accel_magnitude;
        //             uint32_t accel_magnitude_u32;
        //         } float_union;

        //         float_union.accel_magnitude = sqrt(accel_real.x * accel_real.x + accel_real.y * accel_real.y + accel_real.z * accel_real.z);

        //         // output data over uart
        //         char byte_data[4] = {
        //             (float_union.accel_magnitude_u32 >> 0) & 0xFF,
        //             (float_union.accel_magnitude_u32 >> 8) & 0xFF,
        //             (float_union.accel_magnitude_u32 >> 16) & 0xFF,
        //             (float_union.accel_magnitude_u32 >> 24) & 0xFF
        //         };

        //         UART_write(byte_data, 4);

        //         // printf("%f\n", float_union.accel_magnitude);
        //     }

        //     // Give up the semaphore 
        //     xSemaphoreGive(UART_sem);
        // }

        // simulate work
        vTaskDelay(pdTICKS_TO_MS(100));

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
            srand((unsigned int)xTaskGetTickCount());

            // populate data processing buffer
            SPIFFS_packets[packet_index].SPIFFS_file_path = packet->SPIFFS_file_path;
            SPIFFS_packets[packet_index].active = true;
            SPIFFS_packets[packet_index].test_1 = (uint16_t)rand();
            SPIFFS_packets[packet_index].test_2 = (uint16_t)rand();
            SPIFFS_packets[packet_index].test_3 = (uint16_t)rand();

            // spawn data processing thread
            xTaskCreatePinnedToCore(SPIFFS_Write_task, "SPIFFS_Write_task", 4096, (void *)&SPIFFS_packets[packet_index], 1, NULL, 1);
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
            // Create string of passed in data
            char buffer[64];
            sprintf(buffer, "%d,%d,%d\n", packet->test_1, packet->test_2, packet->test_3);

            // Write the data 
            SPIFFS_Write(packet->SPIFFS_file_path, buffer);
            // Can add for easier testing (to make bytes > 247)
            // SPIFFS_Write(packet->SPIFFS_file_path, buffer);
            // SPIFFS_Write(packet->SPIFFS_file_path, buffer);

            // Give up the semaphore 
            packet->active = false;
            xSemaphoreGive(SPIFFS_sem);

            vTaskDelete(NULL);
        }
        
    }
}




void FSM_task(void *args){
    int8_t button_input;
    UBaseType_t stack_watermark;

    state_t state_handler = {
        .current_state = START,
        .next_state = START,
        .skip_button_input = false,
    };

    LED_notify(IDLE);

    while (1)
    {
        // Recieve button input from queue - block if no presses unless state requires skip
        if (state_handler.skip_button_input || (xQueueReceive(Button_queue, &button_input, portMAX_DELAY) == pdTRUE)) {
            stack_watermark = uxTaskGetStackHighWaterMark(NULL);
            ESP_LOGI("FSM_TASK", "Free stack: %u bytes", stack_watermark * sizeof(StackType_t));
            ESP_LOGI("FSM_TASK", "Button input: %d", button_input);

            // state transitions
            switch (state_handler.current_state) {
                case START:

                    switch (button_input) {
                        case HOLD:
                            state_handler.next_state = RESET;
                            state_handler.skip_button_input = true;
                            break;
                        case SINGLE_PRESS:
                            state_handler.next_state = PLAY_SESSION;
                            break;
                        case DOUBLE_PRESS:
                            state_handler.next_state = BLE_SESSION;
                            break;
                        default:
                            state_handler.next_state = state_handler.current_state;
                            break;
                    }

                    break;

                case RESET:

                    // simulate time to reset
                    vTaskDelay(1000);

                    printf("Reset successful\n");
                    state_handler.next_state = START;
                    state_handler.skip_button_input = false;

                    break;

                case PLAY_SESSION:

                    switch (button_input) {
                        case HOLD:
                            state_handler.next_state = RESET;
                            state_handler.skip_button_input = true;
                            break;
                        case SINGLE_PRESS:

                            if (state_handler.Play_Session_Handle != NULL) {

                                xTaskNotify(state_handler.Play_Session_Handle, 0, eNoAction);
                                vTaskDelay(pdMS_TO_TICKS(100)); // Allow time for play session to terminate itself

                                // ensure play session has been deleted
                                eTaskState task_state = eTaskGetState(state_handler.Play_Session_Handle);
                                if (task_state == eDeleted) {
                                    ESP_LOGI(FSM_TAG, "Play_Session_task deleted successfully");

                                    state_handler.Play_Session_Handle = NULL;
                                    state_handler.play_session_active = false;
                                    state_handler.next_state = START;

                                    LED_notify(IDLE);

                                } else {
                                    ESP_LOGE(FSM_TAG, "Play_Session_task still running (state: %d)", task_state);
                                }
                            }
                            else {
                                ESP_LOGE(FSM_TAG,"Play_Session_task exitted abnormally");
                                state_handler.Play_Session_Handle = NULL;
                                state_handler.play_session_active = false;
                                state_handler.next_state = START;

                                LED_notify(IDLE);
                            }
                            break;
                        default:
                            state_handler.next_state = state_handler.current_state;
                            break;
                    }

                    break;

                case BLE_SESSION:

                    switch (button_input) {
                        case HOLD:
                            state_handler.next_state = RESET;
                            state_handler.skip_button_input = true;
                            break;
                        case DOUBLE_PRESS:
                            // End the BLE session
                            BLE_End(); 
                            ESP_LOGI(FSM_TAG,"Ending BLE session");
                            BLE_End(); 
                            LED_notify(IDLE);

                            // Reset SPIFFS file tracking
                            for(int i = 0; i < SPIFFS_files.num_files; i++){
                                SPIFFS_files.file_path[i] = NULL; 
                            }
                            SPIFFS_files.num_files = 0; 
                            
                            // Reset state handler variables
                            state_handler.BLE_session_active = false; 
                            state_handler.next_state = START;
                            break;
                        default:
                            state_handler.next_state = state_handler.current_state;
                            break;
                    }

                    break;

                default:
                    state_handler.next_state = START;
                    break;
            }

            // update state
            state_handler.current_state = state_handler.next_state;

            // handle current state
            printf("Current state: ");
            switch (state_handler.current_state) {
                case START:
                    printf("START\n");
                    break;

                case RESET:
                    printf("RESET\n");
                    break;
                    
                case PLAY_SESSION:
                    printf("PLAY SESSION\n");

                    // spawn play session - only one active at a time
                    if (!state_handler.play_session_active) {

                        if (state_handler.Play_Session_Handle == NULL) {
                            // create session task
                            state_handler.play_session_active = true;
                            xTaskCreatePinnedToCore(Play_Session_task, "Session_task", 4096, NULL, 1, &state_handler.Play_Session_Handle, 0);
                            LED_notify(BATTERY_LEVEL);
                            ESP_LOGI(FSM_TAG,"Play session started");
                        }
                        else {
                            ESP_LOGE(FSM_TAG,"Play_Session_Handle is not null");
                        }
                    }

                    break;

                case BLE_SESSION:
                    printf("BLE SESSION\n");
                    if(!state_handler.BLE_session_active){
                        state_handler.BLE_session_active = true; 
                        BLE_Start(); 
                        LED_notify(BLE_PAIRING);
                    }
                    break;

                default:
                    printf("INVALID\n");
                    break;
            }
        }
    }       
}

void Button_task(void *args) {
    TickType_t curr_time, press_time;
    const TickType_t debounce_time = pdMS_TO_TICKS(30);
    const TickType_t hold_threshold = pdMS_TO_TICKS(400);

    while (1) {
        if (xSemaphoreTake(Button_sem, portMAX_DELAY) == pdTRUE) {
            
            // Debounce button press
            vTaskDelay(debounce_time);
            press_time = xTaskGetTickCount();
            curr_time = press_time;
            // Pause timer if timer running
            if (num_presses != 0) { gptimer_stop(Button_timer); }

            // Poll for hold
            while (!Button_Read())
            {
                // Update current timer
                curr_time = xTaskGetTickCount();
                // Check if press is longer than the hold threshold
                if (curr_time - press_time > hold_threshold) {
                    hold_detected = true;
                    break;
                }
            }

            // Increment press counter if pressed
            num_presses++;
            // Restart the timer if running
            if (num_presses == 0){ gptimer_enable(Button_timer); }
            gptimer_set_raw_count(Button_timer, 0);
            gptimer_start(Button_timer);
            gpio_intr_enable(BUTTON_PIN);
        }
    }
}

/********************************Public Functions***********************************/

/****************************Interrupt Service Routines*****************************/

void Button_ISR() {
    if (!hold_detected)
    {
        // Disable the GPIO interrupt for the button
        gpio_intr_disable(BUTTON_PIN);
        // Give the semaphore to notify button task
        xSemaphoreGiveFromISR(Button_sem, NULL);
    }
}

/****************************Interrupt Service Routines*****************************/