#ifndef _BLE_LOG_H_
#define _BLE_LOG_H_

#include "esp_gattc_api.h"
#include "esp_gap_ble_api.h"

char* gattc_events_to_name(esp_gattc_cb_event_t event);
char* gap_events_to_name (esp_gap_ble_cb_event_t event);
char* gatt_conn_to_name(esp_gatt_conn_reason_t reason);
#endif