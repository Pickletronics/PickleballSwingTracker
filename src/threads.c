// threads.c

/************************************Includes***************************************/

#include "threads.h"

/************************************Includes***************************************/

/********************************Public Variables***********************************/

bool SPI_prints = true;
volatile uint8_t num_presses = 0;
volatile bool hold_detected = false;

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

            // Send data to the BLE data queue 
            if (xQueueSend(data_queue, &mpu.accel.x, 10) != pdPASS) {
                printf("Failed to send accel.x to queue\n");
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
    TickType_t curr_time, press_time;
    const TickType_t debounce_time = pdMS_TO_TICKS(30);
    const TickType_t hold_threshold = pdMS_TO_TICKS(600);

    while (1) {
        if (xSemaphoreTake(Button_sem, portMAX_DELAY) == pdTRUE) {
            // Debounce button press
            vTaskDelay(debounce_time);
            timer_pause(TIMER_GROUP_0, TIMER_0);
            press_time = xTaskGetTickCount();
            curr_time = press_time;

            while (!Button_Read())
            {                
                if (curr_time - press_time > hold_threshold) {
                    hold_detected = true;
                    break;
                }
                curr_time = xTaskGetTickCount();
            }
            
            // Increment press counter
            num_presses++;
            hold_detected = false;

            // Reset timer
            timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0);
            timer_start(TIMER_GROUP_0, TIMER_0);
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

void Button_Timer_ISR(){
    // Stop the timer and reset
    timer_pause(TIMER_GROUP_0, TIMER_0);
    timer_group_clr_intr_status_in_isr(TIMER_GROUP_0, TIMER_0);

    printf("Number of presses detected: %d", num_presses);
}

/****************************Interrupt Service Routines*****************************/