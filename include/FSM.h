// FSM.h

#ifndef FSM_H
#define FSM_H

/************************************Includes***************************************/

#include "button.h"

/************************************Includes***************************************/

/*************************************Defines***************************************/
/*************************************Defines***************************************/

/***********************************Externs*****************************************/
/***********************************Externs*****************************************/

/****************************Data Structure Definitions*****************************/

enum STATE {
    START,
    RESET,
    PLAY_SESSION,
    BLE_SESSION,
    NUM_STATES
};

enum BUTTON_ACTION {
    HOLD = -1,
    SINGLE_PRESS = 1,
    DOUBLE_PRESS = 2,
    NUM_ACTIONS
};

typedef struct state_t {
    enum STATE current_state;
    enum STATE next_state;
    bool skip_button_input;
    bool play_session_active;
    bool BLE_session_active;
    TaskHandle_t Play_Session_Handle;;
} state_t;

/****************************Data Structure Definitions*****************************/

/********************************Public Functions***********************************/

void FSM_task(void *args);

/********************************Public Functions***********************************/



#endif // FSM_H