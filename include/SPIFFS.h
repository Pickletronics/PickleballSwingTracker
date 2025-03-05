// SPIFFS.h

#ifndef SPIFFS_H_
#define SPIFFS_H_

/************************************Includes***************************************/

#include "esp_log.h"
#include "esp_spiffs.h"

#include "LED.h"

/************************************Includes***************************************/

/*************************************Defines***************************************/

#define SPIFFS_TAG "SPIFFS"

/*************************************Defines***************************************/

/***********************************Externs*****************************************/
/***********************************Externs*****************************************/

/********************************Public Functions***********************************/

void SPIFFS_Init(void); 
void SPIFFS_Print(const char *path);
size_t SPIFFS_Dump(const char *path, char *buffer, size_t read_size);
void SPIFFS_Write(const char *path, const char *data);
void SPIFFS_Clear(const char *path);
void SPIFFS_Delete(const char *path);

/********************************Public Functions***********************************/

/****************************Interrupt Service Routines*****************************/
/****************************Interrupt Service Routines*****************************/

#endif // SPIFFS_H