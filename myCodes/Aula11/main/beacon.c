
#include "nvs_flash.h"
#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_bt_main.h"
#include "esp_bt_defs.h"
#include "esp_ibeacon_api.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"

static const char *TAG = "ibeacon";
const char *name = "ESP_RAYSSA";

#define SCAN_RSP   

#ifdef SCAN_RSP
char scan_rsp[31] = {'0'};
#endif

/*
* the bluetooth specs say that you need ADV_SCAN_IND to enable scan responses. 
* After changing that attribute of my ble_adv_params I finally got the scan response to work
* https://www.lucadentella.it/2018/04/03/esp32-35-ble-scan-response/#comment-330017
*/
static esp_ble_adv_params_t ble_adv_params = {
    .adv_int_min = 0x20,
    .adv_int_max = 0x40,
    .adv_type = ADV_TYPE_SCAN_IND,//ADV_TYPE_NONCONN_IND,
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
    .channel_map = ADV_CHNL_ALL,
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

static void ble_gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    switch (event)
    {
    case ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT: // foi completada
        ESP_LOGW(TAG, "ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT");
        #ifndef SCAN_RSP
        esp_ble_gap_start_advertising(&ble_adv_params);
        #endif
        break;

    case ESP_GAP_BLE_ADV_START_COMPLETE_EVT: // começou com sucesso 
        ESP_LOGW(TAG, "ESP_GAP_BLE_ADV_START_COMPLETE_EVT");
        
        if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS)
        {
            ESP_LOGE(TAG, "advertisement failed");
        }
        break;
    
    #ifdef SCAN_RSP
    case ESP_GAP_BLE_SCAN_RSP_DATA_RAW_SET_COMPLETE_EVT:
        ESP_LOGW(TAG, "ESP_GAP_BLE_SCAN_RSP_DATA_RAW_SET_COMPLETE_EVT");
        esp_ble_gap_start_advertising(&ble_adv_params);
        break;
    #endif
    
    default:
        ESP_LOGW(TAG, "%d",event );
        break;
    }
}

void init(void)
{
    //receita de bolo
    nvs_flash_init();
    //não quero usar o bluetooth classico 
    esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT(); 
    esp_bt_controller_init(&bt_cfg);
    esp_bt_controller_enable(ESP_BT_MODE_BLE);

    esp_bluedroid_init();
    esp_bluedroid_enable();

    esp_ble_gap_register_callback(ble_gap_event_handler); //pre conexão 
}

void app_main(void)
{
    init();
    ESP_LOGI(TAG, "started");
    esp_ble_ibeacon_t ibeacon_adv_data;
    esp_init_ibeacon_data(&ibeacon_adv_data);
    
    #ifdef SCAN_RSP
    uint8_t len = strlen(name);
    if (len > 29)
        len = 29;

    scan_rsp[0]= len + 1;
    scan_rsp[1] = 0x09;
    sprintf(&scan_rsp[2], "%.*s", len, name);
    #endif
    //sejam gerados para o Advertising
    esp_ble_gap_config_adv_data_raw((uint8_t*)&ibeacon_adv_data, sizeof(ibeacon_adv_data));
    
    #ifdef SCAN_RSP
    esp_ble_gap_config_scan_rsp_data_raw((uint8_t*)scan_rsp, strlen(name)+2);
    #endif
}
