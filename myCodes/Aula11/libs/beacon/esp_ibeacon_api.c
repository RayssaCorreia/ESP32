/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/



/****************************************************************************
*
* This file is for iBeacon APIs. It supports both iBeacon encode and decode. 
*
* iBeacon is a trademark of Apple Inc. Before building devices which use iBeacon technology,
* visit https://developer.apple.com/ibeacon/ to obtain a license.
*
****************************************************************************/

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "esp_system.h"
#include "esp_gap_ble_api.h"
#include "esp_ibeacon_api.h"
#include "esp_log.h"


const uint8_t uuid_zeros[ESP_UUID_LEN_128] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

/* For iBeacon packet format, please refer to Apple "Proximity Beacon Specification" doc */
/* Constant part of iBeacon data */
esp_ble_ibeacon_head_t ibeacon_common_head = {
    .flags = {0x02, 0x01, 0x06},
    .length = 0x1A,
    .type = 0xFF,
    .company_id = 0x004C,
    .beacon_type = 0x1502
};

/* Vendor part of iBeacon data*/
esp_ble_ibeacon_vendor_t vendor_config = {
    .proximity_uuid = ESP_UUID,
    .measured_power = 0xC0
};

bool esp_ble_is_ibeacon_packet (uint8_t *adv_data, uint8_t adv_data_len){
    bool result = false;

    if ((adv_data != NULL) && (adv_data_len == 0x1E)){
        if (!memcmp(adv_data, (uint8_t*)&ibeacon_common_head, sizeof(ibeacon_common_head))){
            result = true;
        }
    }

    return result;
}

esp_err_t esp_init_ibeacon_data (esp_ble_ibeacon_t *ibeacon_adv_data){
    
    char blemac[6] = {0};
    uint16_t major = 0, minor = 0;
    esp_read_mac((uint8_t *)blemac, ESP_MAC_BT);

    major = ((blemac[3]&0xff)<<8) + (blemac[2]&0xff);
    minor = ((blemac[5]&0xff)<<8) + (blemac[4]&0xff);

    vendor_config.major = major;
    vendor_config.minor = minor;
    ESP_LOGI("TAG", "Device BLE MAC: " MACSTR " ", MAC2STR(blemac));
    esp_ble_ibeacon_vendor_t *vc = &vendor_config;

    ESP_LOGI("TAG", "major=%x = %d", ENDIAN_CHANGE_U16(vc->major),ENDIAN_CHANGE_U16(vc->major));
    ESP_LOGI("TAG", "minor=%x = %d", ENDIAN_CHANGE_U16(vc->minor), ENDIAN_CHANGE_U16(vc->minor));
    if ((vc == NULL) || (ibeacon_adv_data == NULL) || (!memcmp(vc->proximity_uuid, uuid_zeros, sizeof(uuid_zeros)))){
        return ESP_ERR_INVALID_ARG;
    }

    memcpy(&ibeacon_adv_data->ibeacon_head, &ibeacon_common_head, sizeof(esp_ble_ibeacon_head_t));
    memcpy(&ibeacon_adv_data->ibeacon_vendor, vc, sizeof(esp_ble_ibeacon_vendor_t));

    return ESP_OK;
}

