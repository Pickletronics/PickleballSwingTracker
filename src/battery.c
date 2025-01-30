// battery.c

/************************************Includes***************************************/

#include "battery.h"

/************************************Includes***************************************/

/********************************Public Variables***********************************/

adc_oneshot_unit_handle_t adc1_handle;
adc_cali_handle_t adc_cali_handle;

/********************************Public Variables***********************************/

/********************************Static Functions**********************************/

static void led_init(void) {
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
}

static void set_led_status(uint8_t percentage) {
    if (percentage > 50) {
        // Green for good battery (>50%)
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);    // R off
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, 255);  // G on
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2, 0);    // B off
    } else if (percentage > 20) {
        // Yellow for medium battery (20-50%)
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 255);  // R on
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, 255);  // G on
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2, 0);    // B off
    } else {
        // Red for low battery (<20%)
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 255);  // R on
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, 0);    // G off
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2, 0);    // B off
    }
    
    // Update all channels
    for (int i = 0; i < 3; i++) {
        ledc_update_duty(LEDC_LOW_SPEED_MODE, i);
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
    uint32_t actual_voltage = voltage * 2;  // Convert to actual battery voltage
    
    if (actual_voltage >= 3700) return 100;  // Cap at 100% for 3.7V and above
    if (actual_voltage <= 3200) return 0;    // 0% at 3.2V and below
    
    // Linear interpolation between 3.2V and 3.7V
    uint32_t percentage = ((actual_voltage - 3200) * 100) / (3700 - 3200);
    return (uint8_t)percentage;
}

/********************************Static Functions**********************************/

/********************************Public Functions***********************************/

void battery_monitor_init(void) {
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
    adc_cali_line_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT_1,
        .atten = ADC_ATTEN,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_cali_create_scheme_line_fitting(&cali_config, &adc_cali_handle));

    // Initialize RGB LED
    led_init();
}

void battery_monitor_task(void *pvParameters) {
    char msg[50];
    while (1) {
        uint32_t adc_voltage = read_battery_voltage();
        uint32_t batt_voltage = adc_voltage * 2;
        uint8_t percentage = calculate_battery_percentage(adc_voltage);
        
        sprintf(msg, "ADC: %lumV (Battery: %lumV) (%u%%)\r\n", 
            adc_voltage, batt_voltage, percentage);
        UART_write(msg, strlen(msg));
        
        set_led_status(percentage);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

/********************************Public Functions***********************************/