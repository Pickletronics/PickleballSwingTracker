// button.h

#ifndef BUTTON_H_
#define BUTTON_H_

/************************************Includes***************************************/

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include <stdint.h>
#include "driver/gpio.h"
#include "driver/gptimer.h"

/************************************Includes***************************************/

/*************************************Defines***************************************/

#define BUTTON_PIN          GPIO_NUM_16
#define TIMER_RES           1000000                      // 1MHz, 1 tick=1us
#define ALARM_PERIOD_SEC    0.3                          // 300 ms
#define ALARM_COUNT         ALARM_PERIOD_SEC * TIMER_RES // alarm period


/*************************************Defines***************************************/

/********************************Public Functions***********************************/

void Button_Init();
void Button_task(void *args);
bool Button_Timer_Callback(gptimer_handle_t timer, const gptimer_alarm_event_data_t* edata, void* user_data);
void Button_ISR();

/********************************Public Functions***********************************/

#endif // BUTTON_H