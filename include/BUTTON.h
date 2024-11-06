// button.h

#ifndef BUTTON_H_
#define BUTTON_H_

#include "driver/gpio.h"
#include <stdint.h>

#define BUTTON_PIN      GPIO_NUM_34

void Button_Init();
int Button_Read();

#endif // BUTTON_H