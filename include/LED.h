// LED.h
#ifndef LED_H
#define LED_H

/************************************Includes***************************************/

#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
#include <string.h>
#include "esp_log.h"

/************************************Includes***************************************/

/*************************************Defines***************************************/

#define LED_TAG "LED"

// RGB LED GPIO pins (Common Cathode)
#define LED_RED_PIN     25    // A pin
#define LED_GREEN_PIN   26    // C pin
#define LED_BLUE_PIN    27    // B pin
// D pin (longest) connects to GND

// Battery thresholds with 1k/1k divider (divides by 2)
#define BATTERY_MAX     1850  // 3.7V/2
#define BATTERY_MIN     1600  // 3.2V/2

// ADC config
#define BATTERY_ADC_CHANNEL    ADC_CHANNEL_7  // GPIO34
#define ADC_SAMPLES     64
#define ADC_ATTEN      ADC_ATTEN_DB_12

/*************************************Defines***************************************/

/****************************Data Structure Definitions*****************************/

enum LED_STATE {
    START_UP,
    IDLE,
    BLE_PAIRING,
    BLE_PAIRED,
    BLE_TRANSFER,
    BATTERY_LEVEL,
    NUM_LED_STATES
};

/****************************Data Structure Definitions*****************************/

/***********************************Externs*****************************************/

extern adc_oneshot_unit_handle_t adc1_handle;
extern adc_cali_handle_t adc_cali_handle;

/***********************************Externs*****************************************/
/********************************Public Functions***********************************/

void LED_init(void);
void LED_task(void *pvParameters);
void LED_notify(uint32_t notification);

/********************************Public Functions***********************************/





























#endif // LED_H