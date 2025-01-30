// BLE.h

#ifndef BLE_H_
#define BLE_H_

/************************************Includes***************************************/

#include "esp_event.h"
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

#define DEVICE_UUID         0x0180
#define READ_UUID           0xFEF4
#define BLE_PAYLOAD_SIZE    247 
#define BLE_MTU             256

/*************************************Defines***************************************/

/***********************************Externs*****************************************/
/***********************************Externs*****************************************/

/********************************Public Functions***********************************/

void BLE_Start(void);
int BLE_Client_Read(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg); 
void BLE_Advertise(void); 
int BLE_GAP_Event_Handler(struct ble_gap_event *event, void *arg);
void BLE_Sync(void);
void BLE_Launch(void *param);
 
/********************************Public Functions***********************************/

#endif // BLE_H