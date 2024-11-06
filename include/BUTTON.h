// BUTTON.h

#ifndef BUTTON_H_
#define BUTTON_H_

#include "freertos/semphr.h"
#include "freertos/FreeRTOS.h"
#include <stdint.h>
#include "driver/gpio.h"

#define BUTTON_PIN GPIO_NUM_22

void Button_Init();

#endif // BUTTON_H