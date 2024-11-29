// main.c

/************************************Includes***************************************/

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "threads.h"

/************************************Includes***************************************/

/********************************Public Variables***********************************/

SemaphoreHandle_t SPI_sem;
SemaphoreHandle_t Button_sem;
QueueHandle_t Button_queue;

#define QUEUE_LENGTH    2048
#define ITEM_SIZE       sizeof( IMU_sample_t )
static StaticQueue_t xStaticQueue;
QueueHandle_t Sample_queue; 
uint8_t ucQueueStorageArea[ QUEUE_LENGTH * ITEM_SIZE ];

/********************************Public Variables***********************************/

/* Main */
/// Used to initialize peripherals and spawn threads
/// ESP-IDF starts FreeRTOS automatically when function returns
void app_main(void) {
    // Initialize modules
    SPI_Init(); 
    UART_init();
    Button_Init();
    MPU9250_Init();
    BLE_Start(); 
    SPIFFS_init(); 

    // Initialize semaphores
    SPI_sem = xSemaphoreCreateMutex();
    Button_sem = xSemaphoreCreateBinary();

    // Initialize queues
    Sample_queue = xQueueCreateStatic( QUEUE_LENGTH, ITEM_SIZE, ucQueueStorageArea, &xStaticQueue );

    // Spawn threads
    xTaskCreatePinnedToCore(Sample_Sensor_task, "Sample_task", 2048, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(Process_Data_task, "Process_task", 4098, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(Button_task, "Button_task", 2048, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(Timer_task, "timer_task", 2048, NULL, 1, NULL, 1);
    // xTaskCreate(SEM_test, "SEM_TEST", 2048, NULL, 1, NULL);

    // Plot threads
    // xTaskCreate(MPU9250_plot_accel, "Serial_Plot", 2048, NULL, 1, NULL);
    // xTaskCreate(MPU9250_plot_accel_mag, "Serial_Plot", 2048, NULL, 1, NULL);
    // xTaskCreate(MPU9250_plot_gyro, "Serial_Plot", 2048, NULL, 1, NULL);
}
