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

data_processing_packet_t packets[MAX_NUM_PACKETS];

/********************************Public Variables***********************************/

/********************************Public Functions***********************************/

// Sampling task
// Reads accelerometer and gyroscope data from IMU 
// and pushes to buffer for processing
void Sample_Sensor_task(void *args) {
    IMU_sample_t sample;
    uint32_t sample_count = 0;
    const uint32_t NUM_SAMPLES_WAIT = 100;
    const uint32_t NUM_SAMPLES_TOTAL = 200;

    // uint32_t num_samples = 0;
    // TickType_t init_time_sec = xTaskGetTickCount();
    // TickType_t curr_time_sec = xTaskGetTickCount();
    // TickType_t one_sec = pdMS_TO_TICKS(1000);

    // impact settings
    const float IMPACT_CHANGE_THRESHOLD = 20.0f;

    // for demo, led init/vars
    const gpio_num_t LED_PIN = 2;
    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

    Circular_Buffer_Init(IMU_BUFFER);

    while(1){
        // read IMU over SPI
        MPU9250_update();

        // Sample per second check
        // num_samples++;
        // curr_time_sec = xTaskGetTickCount();
        // if (curr_time_sec > init_time_sec + one_sec) {
        //     printf("%ld samples per second\n", num_samples);
        //     num_samples = 0;
        //     init_time_sec = xTaskGetTickCount();
        // }

        // populate sample type
        sample.time = xTaskGetTickCount();
        sample.IMU = mpu;

        // write data
        Circular_Buffer_Write(IMU_BUFFER, sample);

        // check for impact (double tap button to test)

        // Convert raw accelerometer values to m/s^2
        vector3D_t accel_real = {
            sample.IMU.accel.x * SENSITIVITY,
            sample.IMU.accel.y * SENSITIVITY,
            sample.IMU.accel.z * SENSITIVITY
        };

        if (impact_detected) {
            // start counting samples to the right
            sample_count++;
            gpio_set_level(LED_PIN, 1);

            // once desired number of samples, dump buffer and spawn processing thread
            if (sample_count >= NUM_SAMPLES_WAIT) {
                // find free packet
                int8_t packet_index = -1;
                for (int8_t i = 0; i < MAX_NUM_PACKETS; i++) {
                    if (!packets[i].active) {
                        packet_index = i;
                        break;
                    }
                }

                if (packet_index == -1) {
                    printf("Max packets being processed.\n");
                }
                else {
                    // populate data processing buffer
                    packets[packet_index].active = true;
                    packets[packet_index].packet_num = packet_index;
                    packets[packet_index].num_samples = NUM_SAMPLES_TOTAL;
                    packets[packet_index].impact_start_index = NUM_SAMPLES_TOTAL - NUM_SAMPLES_WAIT;
                    packets[packet_index].processing_buffer = Circular_Buffer_Sized_DDump(IMU_BUFFER, NUM_SAMPLES_TOTAL);

                    // spawn data processing thread
                    printf("Sending packet #%d\n", packet_index);
                    xTaskCreatePinnedToCore(Process_Data_task, "Process_task", 2048, (void*)&packets[packet_index], 1, NULL, 1);                 
                }

                // reset necessary variables
                sample_count = 0;
                impact_detected = false;
                gpio_set_level(LED_PIN, 0);
            }
        }
    }   
}

void Process_Data_task(void *args) {

    data_processing_packet_t* packet = (data_processing_packet_t*) args;
    printf("Packet received!\n");

    while (1) {

        // print check for buffer
        // for (uint32_t i = 0; i < packet->num_samples; i++) {
        //     printf("%d\n", packet->processing_buffer[i].IMU.accel.x);
        // }

        // simulate work
        vTaskDelay(2000 + rand() % (8000 - 2000 + 1));

        printf("Freeing packet #%ld\n", packet->packet_num);
        free(packet->processing_buffer);
        packet->active = false;

        vTaskDelete(NULL);
    }
    
}

/*
void Process_Data_task(void *args) {
    const float SENSITIVITY = (4.0f * 9.81f / 32768.0f);
    const float IMPACT_CHANGE_THRESHOLD = 20.0f;
    float prev_accel_magnitude = 0.0f;
    IMU_sample_t sample;

    // FIXME: for demo, led init/vars
    const gpio_num_t LED_PIN = 2;
    const TickType_t LED_on_time = pdMS_TO_TICKS(500);
    TickType_t last_impact_time = 0;
    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

    while(1) {
        // read values from queue
        xQueueReceive( Sample_queue, &sample, 5);

        // Convert raw accelerometer values to m/s^2
        Vector3D accel_real = {
            sample.IMU.accel.x * SENSITIVITY,
            sample.IMU.accel.y * SENSITIVITY,
            sample.IMU.accel.z * SENSITIVITY
        };

        float accel_magnitude = sqrt(accel_real.x * accel_real.x + accel_real.y * accel_real.y + accel_real.z * accel_real.z);
        float magnitude_change = fabs(accel_magnitude - prev_accel_magnitude);

        // Check for impact based on change in magnitude
        if (magnitude_change > IMPACT_CHANGE_THRESHOLD) {
            // Impact detected
            last_impact_time = xTaskGetTickCount();
        }

        // FIXME: for demo, turn on led for 1 sec
        if (xTaskGetTickCount() - last_impact_time < LED_on_time) {
            gpio_set_level(LED_PIN, 1);
        }
        else {
            gpio_set_level(LED_PIN, 0);
        }

        // Update the previous magnitude for the next iteration
        prev_accel_magnitude = accel_magnitude;

        // Dump data for python processing
        if (Dump_data) {
            // Currently a bottleneck - UART is slow
            printf("%02lX,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
            sample.time,
            sample.IMU.accel.x, sample.IMU.accel.y, sample.IMU.accel.z,
            sample.IMU.gyro.x, sample.IMU.gyro.y, sample.IMU.gyro.z,
            sample.IMU.mag.x, sample.IMU.mag.y, sample.IMU.mag.z);
        }

    }
}
*/

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

void Timer_task(void *args){
    int8_t Button_input;

    while (true)
    {
        // Recieve button input from queue
        if (xQueueReceive(Button_queue, &Button_input, pdMS_TO_TICKS(10))) {
            if (Button_input == -1) {
                Dump_data = !Dump_data;
                printf("Hold detected!\n");
            }
            else {
                printf("Number of presses detected: %d\n", Button_input);

                // for testing buffers
                if (Button_input == 2) {
                    impact_detected = true;
                }
            }
        }
        
        vTaskDelay(5);
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