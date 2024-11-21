// button.h

#ifndef BUTTON_H_
#define BUTTON_H_

#include "driver/gpio.h"
#include "driver/gptimer.h"
#include "threads.h"
#include <stdint.h>

#define BUTTON_PIN          GPIO_NUM_34
#define TIMER_RES           1000000                 // 1MHz, 1 tick=1us
#define ALARM_COUNT         ALARM_PERIOD_SEC * 1000000 // alarm period
#define ALARM_PERIOD_SEC    0.4                     // 400 ms

void Button_Init();
int Button_Read();
void Button_Timer_Init();
bool Button_Timer_Callback(gptimer_handle_t timer, const gptimer_alarm_event_data_t* edata, void* user_data);

#endif // BUTTON_H