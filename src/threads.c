// threads.c

/************************************Includes***************************************/

#include "threads.h"

/************************************Includes***************************************/

/********************************Public Variables***********************************/

bool Dump_data = false;

/********************************Public Variables***********************************/

/********************************Public Functions***********************************/

void SEM_test(void *args) {
    TickType_t start, end;

    while (1) {
        if (xSemaphoreTake(SPI_sem, 0) == pdTRUE) {
            start = xTaskGetTickCount();

            // simulate variable work
            vTaskDelay(rand() % 101);

            // release semaphore
            xSemaphoreGive(SPI_sem);
            end = xTaskGetTickCount();

            printf("\nSemaphore held for %.2f seconds\n", ((float)(end-start))/configTICK_RATE_HZ);
        }
        else {}

        vTaskDelay(1);
    }
}

// Sampling task
// Reads accelerometer and gyroscope data from IMU 
// and pushes to buffer for processing
void Sample_Sensor_task(void *args) {
    IMU_sample_t sample;

    while(1){
        // read IMU over SPI
        MPU9250_update();

        // populate sample type
        sample.time = xTaskGetTickCount();
        sample.IMU = mpu;

        // Add sample to queue circularly
        if (uxQueueSpacesAvailable(Sample_queue) == 0) {
            // Queue is full, remove the oldest item
            IMU_sample_t dummy;
            xQueueReceive(Sample_queue, &dummy, 0);
        }
        xQueueSend( Sample_queue, &sample, 0 );
    }   
}

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
            last_impact_time = sample.time;
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

void Button_task(void *args) {
    const TickType_t multipress_time_threshold = pdMS_TO_TICKS(400);
    const TickType_t hold_time_threshold = pdMS_TO_TICKS(600);
    const TickType_t debounce_time = pdMS_TO_TICKS(30);
    TickType_t current_time, prev_press_time;
    bool hold_detected = false;
    int num_presses = 0;
    int button_val;

    while (1) {
        if (xSemaphoreTake(Button_sem, portMAX_DELAY) == pdTRUE) {

            // capture start time of press
            prev_press_time = xTaskGetTickCount();
            current_time = prev_press_time;

            // poll to detect number of presses
            while (current_time - prev_press_time < multipress_time_threshold) {
                
                // update current time
                current_time = xTaskGetTickCount();

                // delay to debounce
                vTaskDelay(debounce_time);

                // read button
                button_val = Button_Read();
                if (!button_val) {
                    prev_press_time = current_time;
                    num_presses++;

                    // wait for button to be released or detect hold
                    while(!Button_Read()) {
                        // update current time
                        current_time = xTaskGetTickCount();

                        // check for hold and break if holding threshold passed
                        if (current_time - prev_press_time > hold_time_threshold) {
                            hold_detected = true;
                            break;
                        }
                    }
                }
            }

            // Handle result
            if (hold_detected) {
                // toggle SPI prints
                Dump_data = !Dump_data;

                printf("Hold detected!\n");
            }
            else {
                printf("Number of presses detected: %d\n", num_presses);
            }
            
            // reset relevant variables
            hold_detected = false;
            num_presses = 0;

            // re-enable button interrupt
            gpio_intr_enable(BUTTON_PIN);
        }
        else {}
    }
}

/********************************Public Functions***********************************/

/****************************Interrupt Service Routines*****************************/

void Button_ISR() {
    // Disable the GPIO interrupt for the button
    gpio_intr_disable(BUTTON_PIN);

    // Give the semaphore to notify button task
    xSemaphoreGiveFromISR(Button_sem, NULL);
}

/****************************Interrupt Service Routines*****************************/