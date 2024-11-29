// SPIFFS.c

/************************************Includes***************************************/

#include "SPIFFS.h"

/************************************Includes***************************************/

/********************************Public Variables***********************************/

// const char *SPIFFS_TAG = "SPIFFS";

/********************************Public Variables***********************************/

/********************************Public Functions***********************************/

void SPIFFS_init() {
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",   
        .partition_label = "spiffs", 
        .max_files = 5,          
        .format_if_mount_failed = true 
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGI(SPIFFS_TAG, "Failed to mount or format SPIFFS");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGI(SPIFFS_TAG, "SPIFFS partition not found");
        } else {
            ESP_LOGE(SPIFFS_TAG,"Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return;
    }
    else {
        ESP_LOGI(SPIFFS_TAG, "SPIFFS initialized correctly.");
    }

    ret = esp_spiffs_check("spiffs");
    if (ret != ESP_OK) {
        ESP_LOGE(SPIFFS_TAG, "SPIFFS check failed (%s). Reformatting...", esp_err_to_name(ret));
        esp_spiffs_format("spiffs");
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info("spiffs", &total, &used);
    if (ret == ESP_OK) {
        ESP_LOGI(SPIFFS_TAG, "total: %d bytes, used: %d bytes", total, used);
    } else {
        ESP_LOGE(SPIFFS_TAG, "Failed to get SPIFFS info (%s)", esp_err_to_name(ret));
    }
}

void SPIFFS_Read(const char *path){
    ESP_LOGI(SPIFFS_TAG, "Reading file");
    FILE* f = fopen(path, "r");
    if (f == NULL) {
        ESP_LOGE(SPIFFS_TAG, "Failed to open file for reading");
        return;
    }
    char buffer[1024];
    size_t bytesRead = fread(buffer, 1, sizeof(buffer) - 1, f);
    buffer[bytesRead] = '\0'; 
    fclose(f);

    ESP_LOGI(SPIFFS_TAG, "Read from file: '%s'", buffer);
    ESP_LOGI(SPIFFS_TAG, "Read %d from file:", bytesRead);
}

void SPIFFS_Write(const char *path, const char *data){
    // First create a file.
    ESP_LOGI(SPIFFS_TAG, "Opening file");
    FILE* f = fopen(path, "w");
    if (f == NULL) {
        ESP_LOGE(SPIFFS_TAG, "Failed to open file for writing");
        return;
    }
    fprintf(f, data);
    fflush(f);
    fclose(f);
    ESP_LOGI(SPIFFS_TAG, "File written");
}

/********************************Public Functions***********************************/

// OLD CODE - May be useful in future
/*
void Dump_Data_task(void *args){
    bool dump_data = false; 

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
} */