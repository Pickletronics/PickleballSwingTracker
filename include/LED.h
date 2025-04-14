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
#define LED_RED_PIN                 GPIO_NUM_0    // A pin
#define LED_GREEN_PIN               GPIO_NUM_1    // C pin
#define LED_BLUE_PIN                GPIO_NUM_2    // B pin
// D pin (longest) connects to GND

// Battery thresholds with 1k/1k divider (divides by 2)
#define BATTERY_MAX                 1850  // 3.7V
#define BATTERY_MIN                 1700  // 3.3V

// ADC config
#define BATTERY_ADC_CHANNEL         ADC_CHANNEL_5
#define ADC_SAMPLES                 64
#define ADC_ATTEN                   ADC_ATTEN_DB_12

// blink control (delay time)
#define SLOW_BLINK                  300
#define MED_BLINK                   100
#define FAST_BLINK                  35

// color intensity control (out of 255)
#define DIM                         30
#define MED                         2*HI/3
#define HI                          255

/*************************************Defines***************************************/

/****************************Data Structure Definitions*****************************/

enum LED_STATE {
    START_UP,
    IDLE,
    BLE_PAIRING,
    BLE_PAIRED,
    BLE_TRANSFER,
    BATTERY_LEVEL,
    SPIFFS_FULL,
    BLINK,
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