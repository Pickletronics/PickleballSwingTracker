// BLE.h

#ifndef BLE_H_
#define BLE_H_

/************************************Includes***************************************/

#include "driver/gpio.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_nimble_hci.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

/************************************Includes***************************************/

/*************************************Defines***************************************/
/*************************************Defines***************************************/

/***********************************Externs*****************************************/
/***********************************Externs*****************************************/

/********************************Public Functions***********************************/

esp_err_t BLE_Init(void);

/********************************Public Functions***********************************/

#endif // BLE_H