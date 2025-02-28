// button.c

/************************************Includes***************************************/

#include "button.h"

/************************************Includes***************************************/

/********************************Public Variables***********************************/

SemaphoreHandle_t Button_sem;
QueueHandle_t Button_queue;

gptimer_handle_t Button_timer;
int8_t num_presses = 0;
volatile bool timer_trig = false;
volatile bool hold_detected = false;

/********************************Public Variables***********************************/

/********************************Static Functions**********************************/

static int Button_Read() {
    return gpio_get_level(BUTTON_PIN);
}

static void Button_Timer_Init(){
    // Initialize button queue
    Button_queue = xQueueCreate(4, sizeof(int8_t));

    // Configure the Timer/Counter for button press detection timing
    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = TIMER_RES, 
    };
    gptimer_new_timer(&timer_config, &Button_timer);

    gptimer_event_callbacks_t cbs = {
        .on_alarm = Button_Timer_Callback,
    };
    gptimer_register_event_callbacks(Button_timer, &cbs, Button_queue);

    gptimer_enable(Button_timer);

    gptimer_alarm_config_t alarm_config = {
        .alarm_count = ALARM_COUNT,
    };
    gptimer_set_alarm_action(Button_timer, &alarm_config);
}

/********************************Static Functions**********************************/

/********************************Public Functions***********************************/

void Button_Init() {
    esp_err_t ret;

    // Configure the button GPIO as input
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_NEGEDGE;      // Trigger on falling edge
    io_conf.pin_bit_mask = (1ULL << BUTTON_PIN);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;    // Enable internal pull-up

    ret = gpio_config(&io_conf);
    if (ret != ESP_OK) {
        // Handle error if ISR service fails to install
        printf("Failed to configure GPIO %d: %s\n", BUTTON_PIN, esp_err_to_name(ret));
    }

    // Install ISR service and add ISR handler
    // ret = gpio_install_isr_service(ESP_INTR_FLAG_EDGE);
    ret = gpio_install_isr_service(0);
    if (ret != ESP_OK) {
        // Handle error if ISR service fails to install
        printf("Failed to install ISR service: %s\n", esp_err_to_name(ret));
    }

    ret = gpio_isr_handler_add(BUTTON_PIN, Button_ISR, NULL);
    if (ret != ESP_OK) {
        // Handle error if ISR handler fails to add
        printf("Failed to add ISR handler: %s\n", esp_err_to_name(ret));
    }

    // Initialize the Timer/Counter for the Button
    Button_Timer_Init();

    Button_sem = xSemaphoreCreateBinary();

    // start button task
    xTaskCreate(Button_task, "Button_task", 2048, NULL, 1, NULL);
}

void Button_task(void *args) {
    TickType_t curr_time, press_time;
    const TickType_t debounce_time = pdMS_TO_TICKS(30);
    const TickType_t hold_threshold = pdMS_TO_TICKS(2000);

    while (1) {
        if (xSemaphoreTake(Button_sem, portMAX_DELAY) == pdTRUE) {
            printf("entering!\n");
            // Debounce button press
            vTaskDelay(debounce_time);
            press_time = xTaskGetTickCount();
            curr_time = press_time;
            // Pause timer if timer running
            if (num_presses != 0) { gptimer_stop(Button_timer); }

            // Poll for hold
            while (!Button_Read())
            {
                // Update current timer
                curr_time = xTaskGetTickCount();
                // Check if press is longer than the hold threshold
                if (curr_time - press_time > hold_threshold) {
                    hold_detected = true;
                    break;
                }
            }

            // Increment press counter if pressed
            num_presses++;
            
            // Debounce button release
            vTaskDelay(debounce_time);

            // Restart the timer if running
            if (num_presses == 0){ gptimer_enable(Button_timer); }
            gptimer_set_raw_count(Button_timer, 0);
            gptimer_start(Button_timer);
            gpio_intr_enable(BUTTON_PIN);
        }
    }
}

/********************************Public Functions***********************************/

/***************************Interrupt Service Routines******************************/

bool IRAM_ATTR Button_Timer_Callback(gptimer_handle_t timer, const gptimer_alarm_event_data_t* edata, void* user_data){
    
    // Stop timer
    gptimer_stop(Button_timer);

    // Write button press type to queue
    BaseType_t high_task_awoken = pdFALSE;
    QueueHandle_t queue = (QueueHandle_t) user_data;
    if (hold_detected) { num_presses = -1; }
    xQueueSendFromISR(queue, &num_presses, &high_task_awoken);

    // Reset relevant variables
    num_presses = 0;
    hold_detected = false;

    return (high_task_awoken == pdTRUE);
    // return true;
}

void IRAM_ATTR Button_ISR() {
    // gpio_set_level(15, 0);
    if (!hold_detected)
    {
        // Disable the GPIO interrupt for the button
        gpio_intr_disable(BUTTON_PIN);
        // Give the semaphore to notify button task
        xSemaphoreGiveFromISR(Button_sem, NULL);
    }

}


/***************************Interrupt Service Routines******************************/