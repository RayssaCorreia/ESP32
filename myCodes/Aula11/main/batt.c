#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_bt.h"

#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_gatt_common_api.h"

#include "sdkconfig.h"
#include "battery_service.h"

#define TAG "batt"
#define SENSOR_NAME "ESP32-RAYSSA"

EventGroupHandle_t events;
#define NOTIFY_ENABLED_BIT BIT0
#define CONNECTED_BIT   BIT1

static int8_t batt = 100;

static esp_attr_value_t sensor_data = {
    .attr_max_len = (uint16_t)sizeof(batt),
    .attr_len = (uint16_t)sizeof(batt),
    .attr_value = (uint8_t *)(&batt),
};

static uint8_t notify_ccc[2] = {0x00, 0x00};
static esp_attr_value_t notify_data = {
    .attr_max_len = (uint16_t)sizeof(notify_ccc),
    .attr_len = (uint16_t)sizeof(notify_ccc),
    .attr_value = (uint8_t *)(notify_ccc),
};

static void gap_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    switch (event)
    {
    case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
        esp_ble_gap_start_advertising(&adv_params);
        break;
    default:
        break;
    }
}

static void gatt_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    switch (event)
    {
    case ESP_GATTS_REG_EVT:
        ESP_LOGW(TAG, "ESP_GATTS_REG_EVT");
        esp_ble_gatts_create_service(gatts_if, &service_def.service_id, GATT_HANDLE_COUNT);
        break;
    case ESP_GATTS_CREATE_EVT:
        ESP_LOGW(TAG, "ESP_GATTS_CREATE_EVT");
        service_def.service_handle = param->create.service_handle;

        esp_ble_gatts_start_service(service_def.service_handle);
        esp_ble_gatts_add_char(service_def.service_handle,
                               &service_def.char_uuid,
                               ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
                               //ESP_GATT_CHAR_PROP_BIT_NOTIFY,
                               ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_NOTIFY,
                               &sensor_data, NULL);
        break;
    case ESP_GATTS_ADD_CHAR_EVT:
    {
        ESP_LOGW(TAG, "ESP_GATTS_ADD_CHAR_EVT");
        service_def.char_handle = param->add_char.attr_handle;
        esp_ble_gatts_add_char_descr(service_def.service_handle, &service_def.descr_uuid,
                                     ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE, &notify_data, NULL);
                                     //ESP_GATT_PERM_WRITE, &notify_data, NULL);
        break;
    }
    case ESP_GATTS_ADD_CHAR_DESCR_EVT:
        ESP_LOGW(TAG, "ESP_GATTS_ADD_CHAR_DESCR_EVT");
        service_def.descr_handle = param->add_char_descr.attr_handle;
        esp_ble_gap_config_adv_data(&adv_data);
        break;

    case ESP_GATTS_CONNECT_EVT:
    {
        ESP_LOGW(TAG, "ESP_GATTS_CONNECT_EVT");
        update_conn_params(param->connect.remote_bda);
        service_def.gatts_if = gatts_if;
        service_def.client_write_conn = param->write.conn_id;
        xEventGroupSetBits(events, CONNECTED_BIT);
        break;
    }

    case ESP_GATTS_READ_EVT:
    {
        ESP_LOGW(TAG, "ESP_GATTS_READ_EVT");
        esp_gatt_rsp_t rsp;
        memset(&rsp, 0, sizeof(esp_gatt_rsp_t));
        rsp.attr_value.handle = param->read.handle;

        if (service_def.descr_handle == param->read.handle)
        {
            ESP_LOGW(TAG, "DESCR HANDLE");
            rsp.attr_value.len = notify_data.attr_len;
            memcpy(rsp.attr_value.value, notify_data.attr_value, notify_data.attr_len);     
        }
        else 
        {
            
            rsp.attr_value.len = sensor_data.attr_len;
            memcpy(rsp.attr_value.value, sensor_data.attr_value, sensor_data.attr_len);

        }
            esp_ble_gatts_send_response(gatts_if, param->read.conn_id, param->read.trans_id,
                        ESP_GATT_OK, &rsp);
        break;
    }
    case ESP_GATTS_WRITE_EVT:
    {
        ESP_LOGW(TAG, "ESP_GATTS_WRITE_EVT");
        if (service_def.descr_handle == param->write.handle)
        {
            uint16_t descr_value = param->write.value[1] << 8 | param->write.value[0];
            memcpy(notify_data.attr_value,param->write.value,2);
            if (descr_value != 0x0000)
            {
                ESP_LOGI(TAG, "notify enable");
                xEventGroupSetBits(events, NOTIFY_ENABLED_BIT);
                esp_ble_gatts_send_indicate(gatts_if, param->write.conn_id, service_def.char_handle,
                                            sensor_data.attr_len, sensor_data.attr_value, false);
            }
            else
            {
                ESP_LOGI(TAG, "notify disable");
                xEventGroupClearBits(events, NOTIFY_ENABLED_BIT);
            }
            esp_ble_gatts_send_response(gatts_if, param->write.conn_id,
                                        param->write.trans_id, ESP_GATT_OK, NULL);
        }
        else
        {
            esp_ble_gatts_send_response(gatts_if, param->write.conn_id,
                                        param->write.trans_id, ESP_GATT_WRITE_NOT_PERMIT, NULL);
        }
        break;
    }

    case ESP_GATTS_DISCONNECT_EVT:
        ESP_LOGW(TAG, "ESP_GATTS_DISCONNECT_EVT");
        service_def.gatts_if = 0;
        esp_ble_gap_start_advertising(&adv_params);
        xEventGroupClearBits(events, CONNECTED_BIT);
        xEventGroupClearBits(events, NOTIFY_ENABLED_BIT);
        break;

    default:
        break;
    }
}

static void read_batt_task(void *arg)
{
    
    while (1)
    {
        vTaskDelay(2000 / portTICK_PERIOD_MS);

        EventBits_t bits = xEventGroupGetBits(events);
        if ( ( bits & NOTIFY_ENABLED_BIT) && (bits & CONNECTED_BIT) ) 
            {
                if(batt-- == 0)
                    batt = 100;
                ESP_LOGI(TAG, "batt: %d", batt);
                esp_ble_gatts_send_indicate(service_def.gatts_if, service_def.client_write_conn,
                                            service_def.char_handle,
                                            sensor_data.attr_len, sensor_data.attr_value, false);
            }

    }
}

void app_main(void)
{
    init_service_def();

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }
    esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    esp_bt_controller_init(&bt_cfg);
    esp_bt_controller_enable(ESP_BT_MODE_BLE);
    esp_bluedroid_init();
    esp_bluedroid_enable();
    esp_ble_gap_set_device_name(SENSOR_NAME);

    esp_ble_gap_register_callback(gap_handler);
    esp_ble_gatts_register_callback(gatt_handler);
    esp_ble_gatts_app_register(0);

    events = xEventGroupCreate();

    xTaskCreate(read_batt_task, "batt", 4098, NULL, 5, NULL);
}
    