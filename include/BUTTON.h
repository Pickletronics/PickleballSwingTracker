// button.h

#ifndef BUTTON_H_
#define BUTTON_H_

#include "driver/gpio.h"
#include "driver/gptimer.h"
#include <stdint.h>

#define BUTTON_PIN      GPIO_NUM_34

extern gptimer_handle_t button_timer;

void Button_Init();
int Button_Read();
void Button_Timer_Init();

#endif // BUTTON_H