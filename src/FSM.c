// FSM.c

/************************************Includes***************************************/

#include "fsm.h"

/************************************Includes***************************************/

/********************************Public Variables***********************************/
/********************************Public Variables***********************************/

/********************************Public Functions***********************************/

void FSM_task(void *args){
    int8_t button_input;
    UBaseType_t stack_watermark;

    state_t state_handler = {
        .current_state = START,
        .next_state = START,
        .skip_button_input = false,
    };

    LED_notify(IDLE);

    while (1)
    {
        // Recieve button input from queue - block if no presses unless state requires skip
        if (state_handler.skip_button_input || (xQueueReceive(Button_queue, &button_input, portMAX_DELAY) == pdTRUE)) {
            stack_watermark = uxTaskGetStackHighWaterMark(NULL);
            ESP_LOGI("FSM_TASK", "Free stack: %u bytes", stack_watermark * sizeof(StackType_t));
            ESP_LOGI("FSM_TASK", "Button input: %d", button_input);

            // state transitions
            switch (state_handler.current_state) {
                case START:

                    switch (button_input) {
                        case HOLD:
                            state_handler.next_state = RESET;
                            state_handler.skip_button_input = true;
                            break;
                        case SINGLE_PRESS:
                            state_handler.next_state = PLAY_SESSION;
                            break;
                        case DOUBLE_PRESS:
                            state_handler.next_state = BLE_SESSION;
                            break;
                        default:
                            state_handler.next_state = state_handler.current_state;
                            break;
                    }

                    break;

                case RESET:

                    // simulate time to reset
                    vTaskDelay(1000);

                    printf("Reset successful\n");
                    state_handler.next_state = START;
                    state_handler.skip_button_input = false;

                    break;

                case PLAY_SESSION:

                    switch (button_input) {
                        case HOLD:
                            state_handler.next_state = RESET;
                            state_handler.skip_button_input = true;
                            break;
                        case SINGLE_PRESS:

                            if (state_handler.Play_Session_Handle != NULL) {

                                xTaskNotify(state_handler.Play_Session_Handle, 0, eNoAction);
                                vTaskDelay(pdMS_TO_TICKS(100)); // Allow time for play session to terminate itself

                                // ensure play session has been deleted
                                eTaskState task_state = eTaskGetState(state_handler.Play_Session_Handle);
                                if (task_state == eDeleted) {
                                    ESP_LOGI(FSM_TAG, "Play_Session_task deleted successfully");

                                    state_handler.Play_Session_Handle = NULL;
                                    state_handler.play_session_active = false;
                                    state_handler.next_state = START;

                                    LED_notify(IDLE);

                                } else {
                                    ESP_LOGE(FSM_TAG, "Play_Session_task still running (state: %d)", task_state);
                                }
                            }
                            else {
                                ESP_LOGE(FSM_TAG,"Play_Session_task exitted abnormally");
                                state_handler.Play_Session_Handle = NULL;
                                state_handler.play_session_active = false;
                                state_handler.next_state = START;

                                LED_notify(IDLE);
                            }
                            break;
                        default:
                            state_handler.next_state = state_handler.current_state;
                            break;
                    }

                    break;

                case BLE_SESSION:

                    switch (button_input) {
                        case HOLD:
                            state_handler.next_state = RESET;
                            state_handler.skip_button_input = true;
                            break;

                        // End the BLE session
                        case DOUBLE_PRESS:
                            ESP_LOGI(FSM_TAG,"Ending BLE session");

                            // Reset SPIFFS file tracking if dumping was initiated 
                            if(BLE_End()) {
                                for(int i = 0; i < SPIFFS_files.num_files; i++){
                                    SPIFFS_Delete(SPIFFS_files.file_path[i]);
                                    free(SPIFFS_files.file_path[i]);
                                    SPIFFS_files.file_path[i] = NULL; 
                                }
                                SPIFFS_files.num_files = 0; 
                            }

                            // Reset state handler variables
                            state_handler.BLE_session_active = false; 
                            state_handler.next_state = START;

                            LED_notify(IDLE);

                            break;

                        default:
                            state_handler.next_state = state_handler.current_state;
                            break;
                    }
                    break;

                default:
                    state_handler.next_state = START;
                    break;
            }

            // update state
            state_handler.current_state = state_handler.next_state;

            // handle current state
            printf("Current state: ");
            switch (state_handler.current_state) {
                case START:
                    printf("START\n");
                    break;

                case RESET:
                    printf("RESET\n");
                    break;
                    
                case PLAY_SESSION:
                    printf("PLAY SESSION\n");

                    // spawn play session - only one active at a time
                    if (!state_handler.play_session_active) {

                        if (state_handler.Play_Session_Handle == NULL) {
                            // create session task
                            state_handler.play_session_active = true;
                            xTaskCreatePinnedToCore(Play_Session_task, "Session_task", 4096, NULL, 1, &state_handler.Play_Session_Handle, 0);
                            LED_notify(BATTERY_LEVEL);
                            ESP_LOGI(FSM_TAG,"Play session started");
                        }
                        else {
                            ESP_LOGE(FSM_TAG,"Play_Session_Handle is not null");
                        }
                    }

                    break;

                case BLE_SESSION:
                    printf("BLE SESSION\n");
                    if(!state_handler.BLE_session_active){
                        state_handler.BLE_session_active = true; 
                        BLE_Start(); 
                        LED_notify(BLE_PAIRING);
                    }
                    break;

                default:
                    printf("INVALID\n");
                    break;
            }
        }
    }       
}

/********************************Public Functions***********************************/