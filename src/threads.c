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
    while (1) {
        if (xSemaphoreTake(Button_sem, portMAX_DELAY) == pdTRUE) {
            if (hold_detected) {
                printf("Hold detected; ignoring further presses.\n");
                continue;  // Skip further processing if a hold is detected
            }

            // Check if the timer interrupt signaled end of a press sequence
            printf("Number of presses detected: %d\n", num_presses);
            num_presses = 0;  // Reset press counter
            gpio_intr_enable(BUTTON_PIN);  // Re-enable button interrupt
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

        // Increment global button press counter
        num_presses++;

        // Restart the timer
        gptimer_stop(button_timer);
        gptimer_set_raw_count(button_timer, 0);
        gptimer_start(button_timer);

        // Give the semaphore to notify button task
        xSemaphoreGiveFromISR(Button_sem, NULL);
    }
}

bool Button_Timer_ISR(){
    // Disable the interrupt
    gptimer_stop(button_timer);    
    // Give the semaphore to notify button task
    xSemaphoreGiveFromISR(Button_sem, NULL);

    return true;
}

/****************************Interrupt Service Routines*****************************/