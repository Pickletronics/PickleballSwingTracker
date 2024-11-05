// BLE.c

/************************************Includes***************************************/

#include "BLE.h"

/************************************Includes***************************************/

/********************************Public Variables***********************************/
/********************************Public Variables***********************************/

/********************************Public Functions***********************************/

esp_err_t BLE_Init(){
    // nvs_flash_init();                          // 1 - Initialize NVS flash using
    // esp_nimble_hci_and_controller_init();      // 2 - Initialize ESP controller
    // nimble_port_init();                        // 3 - Initialize the host stack
    // ble_svc_gap_device_name_set("BLE-Server"); // 4 - Initialize NimBLE configuration - server name
    // ble_svc_gap_init();                        // 4 - Initialize NimBLE configuration - gap service
    // ble_svc_gatt_init();                       // 4 - Initialize NimBLE configuration - gatt service
    // ble_gatts_count_cfg(gatt_svcs);            // 4 - Initialize NimBLE configuration - config gatt services
    // ble_gatts_add_svcs(gatt_svcs);             // 4 - Initialize NimBLE configuration - queues gatt services.
    // ble_hs_cfg.sync_cb = ble_app_on_sync;      // 5 - Initialize application
    return 0; 
}

/********************************Public Functions***********************************/