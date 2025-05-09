// LED.c

/************************************Includes***************************************/

#include "LED.h"

/************************************Includes***************************************/

/********************************Public Variables***********************************/

adc_oneshot_unit_handle_t adc1_handle;
adc_cali_handle_t adc_cali_handle;
TaskHandle_t LED_Handle;

/********************************Public Variables***********************************/

/*********************************Static Functions**********************************/

static void set_RGB(uint8_t R, uint8_t G, uint8_t B) {
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, R);    // R
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, G);    // G
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2, B);    // B
    
    // Update all channels
    for (int i = 0; i < 3; i++) {
        ledc_update_duty(LEDC_LOW_SPEED_MODE, i);
    }
}

static void set_battery_led_status(uint8_t percentage) {
    if (percentage > 20) {
        // Green for good battery (>20%)
        set_RGB(0,HI,0);
    } 
    // else if (percentage > 20) {
    //     // Yellow for medium battery (20-50%)
    //     set_RGB(255,255,0);
    // } 
    else {
        // Red for low battery (<20%)
        set_RGB(HI,0,0);
    }
}

static uint32_t read_battery_voltage(void) {
    int adc_raw;
    int voltage;
    uint32_t adc_reading = 0;
    
    for (int i = 0; i < ADC_SAMPLES; i++) {
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, BATTERY_ADC_CHANNEL, &adc_raw));
        adc_reading += adc_raw;
    }
    adc_reading /= ADC_SAMPLES;
    
    ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc_cali_handle, adc_reading, &voltage));
    return (uint32_t)voltage;
}

static uint8_t calculate_battery_percentage(uint32_t voltage) {
    uint32_t actual_voltage = voltage; //* 2;  // Convert to actual battery voltage
    
    if (actual_voltage >= 3700) return 100;  // Cap at 100% for 3.7V and above
    if (actual_voltage <= 3200) return 0;    // 0% at 3.2V and below
    
    // Linear interpolation between 3.2V and 3.7V
    uint32_t percentage = ((actual_voltage - 3200) * 100) / (3700 - 3200);
    return (uint8_t)percentage;
}

/*********************************Static Functions**********************************/

/********************************Public Functions***********************************/

// RGB LED init
void LED_init(void) {
    // ADC init
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    // ADC config
    adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, BATTERY_ADC_CHANNEL, &config));

    // ADC calibration config
    adc_cali_curve_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT_1,
        .atten = ADC_ATTEN,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_cali_create_scheme_curve_fitting(&cali_config, &adc_cali_handle));

    // Configure LED PWM timer
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .duty_resolution = LEDC_TIMER_8_BIT,
        .freq_hz = 5000,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // Configure LED PWM channels
    ledc_channel_config_t ledc_channel[3] = {
        {   // Red channel
            .channel = LEDC_CHANNEL_0,
            .duty = 0,
            .gpio_num = LED_RED_PIN,
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .hpoint = 0,
            .timer_sel = LEDC_TIMER_0
        },
        {   // Green channel
            .channel = LEDC_CHANNEL_1,
            .duty = 0,
            .gpio_num = LED_GREEN_PIN,
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .hpoint = 0,
            .timer_sel = LEDC_TIMER_0
        },
        {   // Blue channel
            .channel = LEDC_CHANNEL_2,
            .duty = 0,
            .gpio_num = LED_BLUE_PIN,
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .hpoint = 0,
            .timer_sel = LEDC_TIMER_0
        }
    };

    for (int i = 0; i < 3; i++) {
        ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel[i]));
    }

    // start thread
    xTaskCreate(LED_task, "LED_task", 4096, NULL, 1, &LED_Handle);

}

void LED_task(void *pvParameters) {
    enum LED_STATE led_state = START_UP;
    enum LED_STATE prev_state = START_UP;

    // battery status vars
    uint32_t adc_voltage;
    uint8_t percentage = 0;
    
    while (1) {
        uint32_t notification;
        if (xTaskNotifyWait(0, 0, &notification, 0) == pdTRUE) {
            prev_state = led_state;
            led_state = (enum LED_STATE) notification;
            // ESP_LOGI(LED_TAG, "Notification: %ld\n", notification);
        }

        switch (led_state) {
            case START_UP:
                // blink white
                set_RGB(MED,MED,MED);
                vTaskDelay(pdMS_TO_TICKS(SLOW_BLINK));
                set_RGB(DIM,DIM,DIM);
                vTaskDelay(pdMS_TO_TICKS(SLOW_BLINK));
                break;

            case IDLE:
                set_RGB(MED,MED,MED);
                break;

            case BLE_PAIRING:
                // blink blue
                set_RGB(0,0,MED);
                vTaskDelay(pdMS_TO_TICKS(SLOW_BLINK));
                set_RGB(0,0,DIM);
                vTaskDelay(pdMS_TO_TICKS(SLOW_BLINK));
                break;

            case BLE_PAIRED:
                // solid blue
                set_RGB(0,0,MED);
                break;

            case BLE_TRANSFER:
                // rapid blink on/off blue
                set_RGB(0,0,MED);
                vTaskDelay(pdMS_TO_TICKS(FAST_BLINK));
                set_RGB(0,0,0);
                vTaskDelay(pdMS_TO_TICKS(FAST_BLINK));
                break;

            case BATTERY_LEVEL:
                // solid green/yellow/red
                adc_voltage = read_battery_voltage();
                percentage = calculate_battery_percentage(adc_voltage);
                
                // ESP_LOGI(LED_TAG, "ADC: %lumV (%u%%)\r\n", adc_voltage, percentage);
                vTaskDelay(pdMS_TO_TICKS(100));

                set_battery_led_status(percentage);
                break;

            case BLINK:
                set_RGB(0,0,0); // turn off led
                vTaskDelay(pdMS_TO_TICKS(MED_BLINK));
                led_state = prev_state; // set back to prev state
                break;

            case SPIFFS_FULL:
                // blink on/off yellow
                set_RGB(MED,MED,0);
                vTaskDelay(pdMS_TO_TICKS(SLOW_BLINK));
                set_RGB(0,0,0);
                vTaskDelay(pdMS_TO_TICKS(SLOW_BLINK));
                break;
            
            default:
                break;
        }
        
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

void LED_notify(uint32_t notification) {
    xTaskNotify(LED_Handle, notification, eSetValueWithOverwrite);
    // ESP_LOGI(LED_TAG, "Notification sent: %ld\n", notification);
}

/********************************Public Functions***********************************/