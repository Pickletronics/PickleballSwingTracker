// SPIFFS.h

#ifndef SPIFFS_H_
#define SPIFFS_H_

/************************************Includes***************************************/

#include "esp_log.h"
#include "esp_spiffs.h"

/************************************Includes***************************************/

/*************************************Defines***************************************/

#define SPIFFS_TAG "SPIFFS"

/*************************************Defines***************************************/

/***********************************Externs*****************************************/
/***********************************Externs*****************************************/

/********************************Public Functions***********************************/

void SPIFFS_init(void); 
// void SPIFFS_Print(FILE* f);
// bool SPIFFS_Dump(FILE* f, char *buffer);
// void SPIFFS_Write(FILE* f, const char *data);
void SPIFFS_Print(const char *path);
size_t SPIFFS_Dump(const char *path, char *buffer, size_t read_size);
void SPIFFS_Write(const char *path, const char *data);
void SPIFFS_Clear(const char *path);
FILE* SPIFFS_Open_File(const char *path);
bool SPIFFS_Close_File(FILE* f);

/********************************Public Functions***********************************/

/****************************Interrupt Service Routines*****************************/
/****************************Interrupt Service Routines*****************************/

#endif // SPIFFS_H