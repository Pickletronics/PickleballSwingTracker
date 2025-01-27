// threads.c

/************************************Includes***************************************/

#include "threads.h"

/************************************Includes***************************************/

/********************************Public Variables***********************************/

bool Dump_data = false;
volatile bool hold_detected = false;
volatile int8_t num_presses = 0;

// for testing purposes
bool impact_detected = false;

uint8_t active_processing_threads = 0;
uint8_t active_SPIFFS_write_threads = 0;
data_processing_packet_t play_session_packets[MAX_PROCESSING_THREADS];
SPIFFS_packet_t SPIFFS_packets[MAX_SPIFFS_THREADS];

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

    // Check sampling rate
    // uint32_t num_samples = 0;
    // TickType_t init_time_sec = xTaskGetTickCount();
    // TickType_t curr_time_sec = xTaskGetTickCount();
    // TickType_t one_sec = pdMS_TO_TICKS(1000);

    // impact settings
    const float IMPACT_CHANGE_THRESHOLD = 20.0f;
    const TickType_t IMPACT_BUFFER_TIME = pdMS_TO_TICKS(50);
    float prev_accel_magnitude = 0.0f; // FIXME: issue at t=0?

    // demo led init
    const TickType_t LED_on_time = pdMS_TO_TICKS(500);
    TickType_t last_impact_time = 0;
    const gpio_num_t LED_PIN = 2;
    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

    // circular buffer init
    Circular_Buffer_Init(IMU_BUFFER);

    // session SPIFFS init
    const char *file_name = "session_0.txt"; 
    char file_path[64]; 
    sprintf(file_path, "/spiffs/%s", file_name);
    char buffer[64];
    sprintf(buffer, "%s\n", file_name);
    SPIFFS_Write(file_path, buffer);

    while(1){
        // read IMU over SPI
        MPU9250_update();

        // Sample per second check
        /*
        num_samples++;
        curr_time_sec = xTaskGetTickCount();
        if (curr_time_sec > init_time_sec + one_sec) {
            printf("%ld samples per second\n", num_samples);
            num_samples = 0;
            init_time_sec = xTaskGetTickCount();
        }
        */

        // populate sample type
        sample.time = xTaskGetTickCount();
        sample.IMU = mpu;

        // write data
        Circular_Buffer_Write(IMU_BUFFER, sample);

        /* check for impact (triple tap button to test)*/
        // Convert raw accelerometer values to m/s^2
        vector3D_t accel_real = {
            sample.IMU.accel.x * SENSITIVITY,
            sample.IMU.accel.y * SENSITIVITY,
            sample.IMU.accel.z * SENSITIVITY
        };
        float accel_magnitude = sqrt(accel_real.x * accel_real.x + accel_real.y * accel_real.y + accel_real.z * accel_real.z);
        float magnitude_change = fabs(accel_magnitude - prev_accel_magnitude);

        // Check for impact based on change in magnitude (ignore other "impacts" for IMPACT_BUFFER_TIME ms)
        if ((magnitude_change > IMPACT_CHANGE_THRESHOLD) && (xTaskGetTickCount() - last_impact_time > IMPACT_BUFFER_TIME)) {
            // Impact detected
            last_impact_time = xTaskGetTickCount();
            impact_detected = true;
            // printf("Imapct at %ld\n", last_impact_time);
        }

        // FIXME: for demo, turn on led for 1/2 sec
        if (xTaskGetTickCount() - last_impact_time < LED_on_time) {
            gpio_set_level(LED_PIN, 1);
        }
        else {
            gpio_set_level(LED_PIN, 0);
        }

        // Update the previous magnitude for the next iteration
        prev_accel_magnitude = accel_magnitude;

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
                    printf("Max play session packets being processed.\n");
                }
                else {
                    // populate data processing buffer
                    play_session_packets[packet_index].SPIFFS_file_path = file_path;
                    play_session_packets[packet_index].active = true;
                    play_session_packets[packet_index].num_samples = NUM_SAMPLES_TOTAL;
                    play_session_packets[packet_index].impact_start_index = NUM_SAMPLES_TOTAL - NUM_SAMPLES_WAIT;
                    play_session_packets[packet_index].processing_buffer = Circular_Buffer_Sized_DDump(IMU_BUFFER, NUM_SAMPLES_TOTAL);

                    // spawn data processing thread
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

    active_processing_threads++;
    data_processing_packet_t* packet = (data_processing_packet_t*) args;

    while (1) {

        // acquire UART sem
        /*
        if (xSemaphoreTake(UART_sem, portMAX_DELAY) == pdTRUE) {    
            for (uint32_t i = 0; i < packet->num_samples; i++) {

                // Convert raw accelerometer values to m/s^2
                vector3D_t accel_real = {
                    packet->processing_buffer[i].IMU.accel.x * SENSITIVITY,
                    packet->processing_buffer[i].IMU.accel.y * SENSITIVITY,
                    packet->processing_buffer[i].IMU.accel.z * SENSITIVITY
                };

                union {
                    float accel_magnitude;
                    uint32_t accel_magnitude_u32;
                } float_union;

                float_union.accel_magnitude = sqrt(accel_real.x * accel_real.x + accel_real.y * accel_real.y + accel_real.z * accel_real.z);

                // output data over uart
                char byte_data[4] = {
                    (float_union.accel_magnitude_u32 >> 0) & 0xFF,
                    (float_union.accel_magnitude_u32 >> 8) & 0xFF,
                    (float_union.accel_magnitude_u32 >> 16) & 0xFF,
                    (float_union.accel_magnitude_u32 >> 24) & 0xFF
                };

                UART_write(byte_data, 4);

                // printf("%f\n", float_union.accel_magnitude);
            }

            // Give up the semaphore 
            xSemaphoreGive(UART_sem);
        }
        */

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

        ESP_LOGI(PROCESSING_TAG, "Deleting processing thread | # Active: (%d)", --active_processing_threads);
        vTaskDelete(NULL);
    }
    
}

/*
void SPIFFS_Test_task(void *args){
    // Create variables for SPIFFS
    const char *file_path = "/spiffs/session_0.txt"; 
    SPIFFS_Clear(file_path);

    // Instantiate testing struct 
    SPIFFS_packet_t test_data;
    test_data.blah = 0;
    test_data.blah2 = 1; 
    test_data.blah3 = 2; 
    
    while(1){
        // "Randomize" the struct data 
        test_data.blah++;
        test_data.blah2++;
        test_data.blah3++;

        // Spawn new thread 
        xTaskCreatePinnedToCore(SPIFFS_Write_task, "SPIFFS_Write_task", 4096, (void *)&test_data, 1, NULL, 0);
        xTaskCreatePinnedToCore(SPIFFS_Write_task, "SPIFFS_Write_task", 4096, (void *)&test_data, 1, NULL, 0);

        vTaskDelay(1000);
    }
}
*/

void SPIFFS_Write_task(void *args){
    // Get the passed in data
    SPIFFS_packet_t *packet = (SPIFFS_packet_t *)args; 

    while(1){

        if (xSemaphoreTake(SPIFFS_sem, portMAX_DELAY) == pdTRUE) {
            // Create string of passed in data
            char buffer[64];
            sprintf(buffer, "%d,%d,%d\n", packet->test_1, packet->test_2, packet->test_3);

            // // Write the data 
            SPIFFS_Write(packet->SPIFFS_file_path, buffer);
            // SPIFFS_Print(packet->SPIFFS_file_path); // For testing

            // Give up the semaphore 
            packet->active = false;
            xSemaphoreGive(SPIFFS_sem);

            vTaskDelete(NULL);
        }
        
    }
}




void FSM_task(void *args){
    int8_t button_input;

    state_t state_handler = {
        .current_state = START,
        .next_state = START,
        .skip_button_input = false,
    };

    while (1)
    {
        // Recieve button input from queue - block if no presses unless state requires skip
        if (state_handler.skip_button_input || (xQueueReceive(Button_queue, &button_input, portMAX_DELAY) == pdTRUE)) {
            printf("Button input: %d\n", button_input);

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
                            vTaskDelete(state_handler.task_handles.Play_Session_Handle);
                            state_handler.task_handles.Play_Session_Handle = NULL;
                            state_handler.play_session_active = false;
                            state_handler.next_state = START;

                            ESP_LOGI(FSM_TAG,"Play session ended");
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
                            printf("Ending BLE session\n");
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

                        if (state_handler.task_handles.Play_Session_Handle == NULL) {
                            xTaskCreatePinnedToCore(Play_Session_task, "Session_task", 4096, &state_handler.task_handles, 1, &state_handler.task_handles.Play_Session_Handle, 0);
                            state_handler.play_session_active = true;
                            ESP_LOGI(FSM_TAG,"Play session started");
                        }
                        else {
                            ESP_LOGE(FSM_TAG,"Play_Session_Handle is not null");
                        }
                    }

                    break;

                case BLE_SESSION:
                    printf("BLE SESSION\n");
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