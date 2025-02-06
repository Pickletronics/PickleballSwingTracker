// BLE.c

/************************************Includes***************************************/

#include "BLE.h"
#include "threads.h"

/************************************Includes***************************************/

/********************************Public Variables***********************************/

char *BLE_NAME = "BLE-Server";
uint8_t ble_addr_type;
uint8_t curr_session = 0;

static const struct ble_gatt_svc_def gatt_svcs[] = {
    {.type = BLE_GATT_SVC_TYPE_PRIMARY,
     .uuid = BLE_UUID16_DECLARE(DEVICE_UUID),                 // Define UUID for client type
     .characteristics = (struct ble_gatt_chr_def[]){
         {.uuid = BLE_UUID16_DECLARE(READ_UUID),           // Define UUID for reading
          .flags = BLE_GATT_CHR_F_READ,
          .access_cb = BLE_Client_Read},
         {0}}},
    {0}};

/********************************Public Variables***********************************/

/********************************Public Functions***********************************/

void BLE_Start(){
    // Initialize necessary modules 
    nvs_flash_init();                          // 1 - Initialize NVS flash using
    nimble_port_init();                        // 3 - Initialize the host stack
    
    // Set up GAP and GATT services 
    ble_svc_gap_device_name_set(BLE_NAME);          // 4 - Initialize NimBLE configuration - server name
    ble_svc_gap_init();                        // 4 - Initialize NimBLE configuration - gap service
    ble_svc_gatt_init();                       // 4 - Initialize NimBLE configuration - gatt service
    ble_gatts_count_cfg(gatt_svcs);            // 4 - Initialize NimBLE configuration - config gatt services
    ble_gatts_add_svcs(gatt_svcs);             // 4 - Initialize NimBLE configuration - queues gatt services.

    // Set the preferred MTU size (controls how many bytes can be sent at once)
    ble_att_set_preferred_mtu(BLE_MTU); 

    // Set the sync callback 
    ble_hs_cfg.sync_cb = BLE_Sync;      // 5 - Initialize application

    // Run the thread with RTOS 
    nimble_port_freertos_init(BLE_Launch);   
}

// Most parameters are unused in our case because we only have one connection, 
// one characteristic, and aren't passing in extra args right now. 
// Need these parameters to be compliant with the NimBLE API. 
int BLE_Client_Read(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg){
    // Make sure there is a valid session to read
    LED_notify(BLE_TRANSFER);
    if(curr_session < SPIFFS_files.num_files){
        // Create buffer to store SPIFFS data
        char *buffer = malloc(BLE_PAYLOAD_SIZE);
        size_t bytesRead = SPIFFS_Dump(SPIFFS_files.file_path[curr_session], buffer, BLE_PAYLOAD_SIZE);
        printf("Read %zu bytes: %.*s\n", bytesRead, (int)bytesRead, buffer); // Debugging

        // If file was not empty (or fully read), send the data read
        if(bytesRead > 0){
            os_mbuf_append(ctxt->om, buffer, bytesRead);
        }
        // Otherwise send empty file message and move to next session if applicable.
        else{
            free(SPIFFS_files.file_path[curr_session]);
            curr_session++; 
            os_mbuf_append(ctxt->om, "End of file reached.", sizeof("End of file reached."));
        }
        free(buffer);
    }
    // Else you have read all sessions.
    else {
        os_mbuf_append(ctxt->om, "Dumped all sessions.", sizeof("Dumped all sessions."));
        LED_notify(BLE_PAIRED);
    }
   
    return 0; 
}

void BLE_Advertise(){
    // GAP - device name definition
    struct ble_hs_adv_fields fields;
    memset(&fields, 0, sizeof(fields));

    fields.name = (uint8_t *)ble_svc_gap_device_name();
    fields.name_len = strlen(ble_svc_gap_device_name());
    fields.name_is_complete = 1;

    ble_gap_adv_set_fields(&fields);

    // GAP - device connectivity definition 
    struct ble_gap_adv_params adv_params;
    memset(&adv_params, 0, sizeof(adv_params));

    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;   // Makes the ESP generally discoverable and connectable 
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;

    ble_gap_adv_start(ble_addr_type, NULL, BLE_HS_FOREVER, &adv_params, BLE_GAP_Event_Handler, NULL); // Start advertising
}

int BLE_GAP_Event_Handler(struct ble_gap_event *event, void *arg){
    switch(event->type){
    // Advertise to find a connection
    case BLE_GAP_EVENT_CONNECT: 
        if(event->connect.status != 0){
            BLE_Advertise(); 
            break; 
        }
        LED_notify(BLE_PAIRED);
        ESP_LOGI(BLE_TAG, "Connected Successfully!"); 
        break; 

    case BLE_GAP_EVENT_DISCONNECT: 
        ESP_LOGI(BLE_TAG, "Connection terminated. Advertising again.");
        BLE_Advertise(); 
        break;

    // Advertise again after completion (timeout)
    case BLE_GAP_EVENT_ADV_COMPLETE: 
        ESP_LOGI(BLE_TAG, "Advertising event timed out. Advertising again.");
        BLE_Advertise(); 
        break; 

    default: 
        break; 
    }
    return 0; 
}

void BLE_Sync(){
    ble_hs_id_infer_auto(0, &ble_addr_type);
    BLE_Advertise();                    
}

void BLE_Launch(void *param){
    nimble_port_run(); 
}

void BLE_End(){
    // Stop advertising if it's running
    if (ble_gap_adv_active()) {
        ble_gap_adv_stop();
    }
    ESP_LOGI(BLE_TAG, "advertising stopped.");

    // Shutdown the NimBLE host
    nimble_port_deinit();

    ESP_LOGI(BLE_TAG, "Bluetooth session terminated.");
}
/********************************Public Functions***********************************/