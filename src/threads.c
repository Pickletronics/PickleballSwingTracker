// threads.c

/************************************Includes***************************************/

#include "threads.h"

/************************************Includes***************************************/

/********************************Public Variables***********************************/

bool SPI_prints = false;
bool dump_data = false; 

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

            if (SPI_prints)
                printf("\nSemaphore held for %.2f seconds\n", ((float)(end-start))/configTICK_RATE_HZ);
        }
        else {}

        vTaskDelay(1);
    }
}

void SPI_test(void *args) {
    uint8_t who_am_i;

    while(1){
        // try to acquire semaphore for 10 ticks
        if (xSemaphoreTake(SPI_sem, 10) == pdTRUE) {

            // read MPU9250 over SPI
            who_am_i = MPU9250_read_WHOAMI();
            MPU9250_update();

            // release semaphore
            xSemaphoreGive(SPI_sem);

            // Send data to the data queue 
            if (xQueueSend(data_queue, &mpu.accel.x, 10) != pdPASS) {
                printf("Failed to send accel.x to queue\n");
                
                // Signal the semaphore for data 
                dump_data = true; 
            }
            if (xQueueSend(data_queue, &mpu.accel.y, 10) != pdPASS) {
                printf("Failed to send accel.y to queue\n");
            }
            if (xQueueSend(data_queue, &mpu.accel.z, 10) != pdPASS) {
                printf("Failed to send accel.z to queue\n");
            }

            // print data
            if (SPI_prints) {
                printf("\n");
                printf("Who Am I: 0x%02X\n", who_am_i);
                printf("\n");
                printf("Accel.x = %d\n", mpu.accel.x);
                printf("Accel.y = %d\n", mpu.accel.y);
                printf("Accel.z = %d\n", mpu.accel.z);
                printf("\n");
                printf("Gyro.x = %d\n", mpu.gyro.x);
                printf("Gyro.y = %d\n", mpu.gyro.y);
                printf("Gyro.z = %d\n", mpu.gyro.z);
            }
        }
        else {}

        vTaskDelay(1); 
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
                printf("Hold detected!\n");

                // toggle SPI prints
                SPI_prints = !SPI_prints;
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

void DumpData_task(void *args){
    // Create variables for SPIFFS
    const char *file_path = "/spiffs/data.csv"; 

    // Open the file to dump to
    FILE *file = fopen(file_path, "w");
    if (file == NULL) {
        ESP_LOGE(SPIFFS_TAG, "Failed to open file for writing");
        return;
    }

    // Send CSV header
    fprintf(file, "Tick Count,Accel X,Accel Y,Accel Z,Gyro X,Gyro Y,Gyro Z,Magno X,Magno Y,Magno Z\n");
    fflush(file);

    // Create variables to store data values 
    float current_time; 
    int16_t sensor_vals[9];

    // Create buffer for data 
    int16_t data;

    while(1){
        // Wait for signal to dump data and take control of SPI
        if (dump_data && xSemaphoreTake(SPI_sem, 10) == pdTRUE) {
            // Read items in the queue 
            int i = 0;
            while(xQueueReceive(data_queue, &data, 0) == pdTRUE){
                sensor_vals[i++] = data; 
                // TODO: Update to 9 once magno values are being read 
                if(i == 6) {
                     // Get current tick count
                    current_time = (float)xTaskGetTickCount() / configTICK_RATE_HZ;
                    
                    fprintf(file, "%.2f,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                        current_time,
                        sensor_vals[0], sensor_vals[1], sensor_vals[2],
                        sensor_vals[3], sensor_vals[4], sensor_vals[5],
                        0, 0, 0); // TODO: Update to use sensor value once magno is read
                    fflush(file);
                    i = 0; 
                }
            }
            // Release the SPI semaphore
            xSemaphoreGive(SPI_sem); 

            // Close file 
            fclose(file);
            SPIFFS_Read(file_path);
             
            // Kill self 
            vTaskDelete(NULL); 
        }
        else {}
        vTaskDelay(1); 
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