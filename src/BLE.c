// BLE.c

/************************************Includes***************************************/

#include "BLE.h"
#include "threads.h"

/************************************Includes***************************************/

/********************************Public Variables***********************************/

char *BLE_TAG = "BLE-Server";
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
    // // Initialize NVS 
    // esp_err_t ret = nvs_flash_init();
    // if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    //     ESP_ERROR_CHECK(nvs_flash_erase());
    //     ret = nvs_flash_init();
    // }
    // ESP_ERROR_CHECK(ret);

    // Initialize necessary modules 
    nvs_flash_init();                          // 1 - Initialize NVS flash using
    nimble_port_init();                        // 3 - Initialize the host stack
    
    // Set up GAP and GATT services 
    ble_svc_gap_device_name_set(BLE_TAG);          // 4 - Initialize NimBLE configuration - server name
    ble_svc_gap_init();                        // 4 - Initialize NimBLE configuration - gap service
    ble_svc_gatt_init();                       // 4 - Initialize NimBLE configuration - gatt service
    ble_gatts_count_cfg(gatt_svcs);            // 4 - Initialize NimBLE configuration - config gatt services
    ble_gatts_add_svcs(gatt_svcs);             // 4 - Initialize NimBLE configuration - queues gatt services.

    // Set the preferred MTU size (controls how many bytes can be sent at once)
    ble_att_set_preferred_mtu(BLE_PAYLOAD_SIZE); 

    // Set the sync callback 
    ble_hs_cfg.sync_cb = BLE_Sync;      // 5 - Initialize application

    // Run the thread with RTOS 
    nimble_port_freertos_init(BLE_Launch);   
}

// Most parameters are unused in our case because we only have one connection, 
// one characteristic, and aren't passing in extra args right now. 
// Need these parameters to be compliant with the NimBLE API. 
int BLE_Client_Read(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg){
    // printf("%d\n", curr_session);
    // printf("%d\n", SPIFFS_files.num_files); 
    // printf("%s\n", SPIFFS_files.file_path[curr_session]);
    if(curr_session < SPIFFS_files.num_files){
        // Create buffer to store SPIFFS data
        char *buffer = malloc(BLE_PAYLOAD_SIZE);
        size_t bytesRead = SPIFFS_Dump(SPIFFS_files.file_path[curr_session], buffer, BLE_PAYLOAD_SIZE);
        printf("Read %zu bytes: %.*s\n", bytesRead, (int)bytesRead, buffer); // Debugging

        // If file was not empty (or fully read), send the bytes read
        if(bytesRead > 0){
            os_mbuf_append(ctxt->om, buffer, bytesRead);
        }
        // Otherwise send empty file message and move to next file if applicable.
        else{
            curr_session++; 
            os_mbuf_append(ctxt->om, "End of file reached.", sizeof("End of file reached."));
        }
        free(buffer);
    }
    else {
        os_mbuf_append(ctxt->om, "Dumped all sessions.", sizeof("Dumped all sessions."));
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
    // If connected, advertise 
    case BLE_GAP_EVENT_CONNECT: 
        if(event->connect.status != 0){
            BLE_Advertise(); 
            // printf("Connection failed."); 
            break; 
        }
        printf("Connected Successfully!\n"); 
        break; 

    case BLE_GAP_EVENT_DISCONNECT: 
        printf("Connection terminated. Ending bluetooth session.\n"); 
        nimble_port_stop(); 
        break;

    // Advertise again after completion 
    // case BLE_GAP_EVENT_ADV_COMPLETE: 
    //     printf("Advertising event complete. Advertising again."); 
    //     BLE_Advertise(); 
    //     break; 

    default: 
        break; 
    }
    return 0; 
}

// int mtu_event_handler(struct ble_gap_event *event, void *arg) {
//     if (event->type == BLE_GAP_EVENT_MTU) {
//         printf("Negotiated MTU: %d\n", event->mtu.value);
//     }
//     return 0;
// }

void BLE_Sync(){
    ble_hs_id_infer_auto(0, &ble_addr_type);
    BLE_Advertise();    
    // ble_gap_event_listener_register(mtu_event_handler, NULL); // Register MTU callback                
}

void BLE_Launch(void *param){
    nimble_port_run(); 
}

/********************************Public Functions***********************************/