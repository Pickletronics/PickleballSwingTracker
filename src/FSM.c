// FSM.c

/************************************Includes***************************************/

#include "FSM.h"

/************************************Includes***************************************/

/********************************Public Variables***********************************/
/********************************Public Variables***********************************/

/********************************Static Functions**********************************/

// START state control functions
static void START_Enter(int8_t button_input, FSM_state_t* state_handler) {
    printf("START\n");
}

static void START_Transition(int8_t button_input, FSM_state_t* state_handler) {
    switch (button_input) {
        case HOLD:
            state_handler->next_state = RESET;
            break;
        case SINGLE_PRESS:
            state_handler->next_state = PLAY_SESSION;
            break;
        case DOUBLE_PRESS:
            state_handler->next_state = BLE_SESSION;
            break;
        default:
            state_handler->next_state = state_handler->current_state;
            break;
    }
}


// PLAY_SESSION state control functions
static void PLAY_SESSION_Enter(int8_t button_input, FSM_state_t* state_handler) {
    printf("PLAY SESSION\n");

    // spawn play session - only one active at a time
    if (!state_handler->play_session_active) {

        if (state_handler->Play_Session_Handle == NULL) {
            // create session task
            state_handler->play_session_active = true;
            LED_notify(BATTERY_LEVEL);
            xTaskCreate(Play_Session_task, "Session_task", 4096, NULL, 1, &state_handler->Play_Session_Handle);
            ESP_LOGI(FSM_TAG,"Play session started");
        }
        else {
            ESP_LOGE(FSM_TAG,"Play_Session_Handle is not null");
        }
    }
}

static void PLAY_SESSION_Transition(int8_t button_input, FSM_state_t* state_handler) {
    switch (button_input) {
        case HOLD:
            state_handler->next_state = RESET;
            break;
        case SINGLE_PRESS:

            if (state_handler->Play_Session_Handle != NULL) {

                xTaskNotify(state_handler->Play_Session_Handle, 0, eNoAction);
                vTaskDelay(pdMS_TO_TICKS(100)); // Allow time for play session to terminate itself

                // ensure play session has been deleted
                eTaskState task_state = eTaskGetState(state_handler->Play_Session_Handle);
                if (task_state == eDeleted) {
                    ESP_LOGI(FSM_TAG, "Play_Session_task deleted successfully");

                    state_handler->Play_Session_Handle = NULL;
                    state_handler->play_session_active = false;
                    state_handler->next_state = START;

                    LED_notify(IDLE);

                } else {
                    ESP_LOGE(FSM_TAG, "Play_Session_task still running (state: %d)", task_state);
                }
            }
            else {
                ESP_LOGE(FSM_TAG,"Play_Session_task exitted abnormally");
                state_handler->Play_Session_Handle = NULL;
                state_handler->play_session_active = false;
                state_handler->next_state = START;

                LED_notify(IDLE);
            }
            break;
        default:
            state_handler->next_state = state_handler->current_state;
            break;
    }
}


// BLE_SESSION state control functions
static void BLE_SESSION_Enter(int8_t button_input, FSM_state_t* state_handler) {
    printf("BLE SESSION\n");
    if(!state_handler->BLE_session_active){
        state_handler->BLE_session_active = true; 
        BLE_Start(); 
        LED_notify(BLE_PAIRING);
    }
}

static void BLE_SESSION_Transition(int8_t button_input, FSM_state_t* state_handler) {
    switch (button_input) {
        case HOLD:
            state_handler->next_state = RESET;
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
            state_handler->BLE_session_active = false; 
            state_handler->next_state = START;

            LED_notify(IDLE);

            break;

        default:
            state_handler->next_state = state_handler->current_state;
            break;
    }
}

/********************************Static Functions**********************************/

/********************************Public Functions***********************************/

void FSM_task(void *args){
    int8_t button_input;
    UBaseType_t stack_watermark;
    FSM_state_t state_handler = {
        .current_state = START,
        .next_state = START,
    };

    LED_notify(IDLE);

    while (1)
    {
        // Recieve button input from queue - block if no presses
        if (xQueueReceive(Button_queue, &button_input, portMAX_DELAY) == pdTRUE) {

            // monitor FSM task stack growth (make sure does not get close to exceeding stack)
            stack_watermark = uxTaskGetStackHighWaterMark(NULL);
            ESP_LOGV("FSM_TASK", "Unused stack: %u bytes", stack_watermark * sizeof(StackType_t));

            // state transitions
            ESP_LOGI("FSM_TASK", "Button input: %d", button_input);
            switch (state_handler.current_state) {
                case START:
                    START_Transition(button_input, &state_handler);
                    break;

                case PLAY_SESSION:
                    PLAY_SESSION_Transition(button_input, &state_handler);
                    break;

                case BLE_SESSION:
                    BLE_SESSION_Transition(button_input, &state_handler);
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
                    START_Enter(button_input, &state_handler);
                    break;

                case RESET:
                    // Software reset of system - should reinit FSM back to START
                    printf("RESET\n");
                    esp_restart();
                    break;
                    
                case PLAY_SESSION:
                    PLAY_SESSION_Enter(button_input, &state_handler);
                    break;

                case BLE_SESSION:
                    BLE_SESSION_Enter(button_input, &state_handler);
                    break;

                default:
                    printf("INVALID\n");
                    break;
            }
        }
    }       
}

/********************************Public Functions***********************************/