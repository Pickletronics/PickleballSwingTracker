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