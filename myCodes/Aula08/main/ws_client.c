/* ESP HTTP Client Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/


#include <stdio.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "protocol_examples_common.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"

#include "esp_log.h"
#include "esp_websocket_client.h"
#include "esp_event.h"
#include "sdkconfig.h"

#define SERVER_URI  "ws://172.16.107.176"
#define SERVER_PORT 8000
#define SERVER_PATH "/websocket"

static const char *TAG_WEBSOCKET = "WEBSOCKET";

const char* opCodeToString(char num) {
	switch(num){
		case 0x00: return "continuation";
		case 0x01: return "text";
		case 0x02: return "binary";
		case 0x08: return "connclose";
		case 0x09: return "ping";
		case 0x0A: return "pong";
	default: return "undefined";
	}
}

static void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;
    switch (event_id) {
    case WEBSOCKET_EVENT_CONNECTED:
        ESP_LOGI(TAG_WEBSOCKET, "WEBSOCKET_EVENT_CONNECTED");
        break;
    case WEBSOCKET_EVENT_DISCONNECTED:
        ESP_LOGI(TAG_WEBSOCKET, "WEBSOCKET_EVENT_DISCONNECTED");
        break;
    case WEBSOCKET_EVENT_DATA:
        ESP_LOGI(TAG_WEBSOCKET, "WEBSOCKET_EVENT_DATA");
        ESP_LOGI(TAG_WEBSOCKET, "Received opcode=%s", opCodeToString(data->op_code));
        if (data->op_code == 0x08) {
            ESP_LOGW(TAG_WEBSOCKET, "Received closed message with code=%d", 256*data->data_ptr[0] + data->data_ptr[1]);
        } else {
            if(data->data_len > 0)
                ESP_LOGW(TAG_WEBSOCKET, "Received=%.*s", data->data_len, (char *)data->data_ptr);
        }
        ESP_LOGW(TAG_WEBSOCKET, "Total payload length=%d, data_len=%d, current payload offset=%d\r\n", data->payload_len, data->data_len, data->payload_offset);

        break;
    case WEBSOCKET_EVENT_ERROR:
        ESP_LOGI(TAG_WEBSOCKET, "WEBSOCKET_EVENT_ERROR");
        break;
    }
}

static void websocket_app_start(esp_websocket_client_handle_t *client)
{
    esp_websocket_client_config_t websocket_cfg = {};

    websocket_cfg.uri = SERVER_URI;
    websocket_cfg.port = SERVER_PORT;
    websocket_cfg.path = SERVER_PATH;

    ESP_LOGI(TAG_WEBSOCKET, "Connecting to %s...", websocket_cfg.uri);

    *client = esp_websocket_client_init(&websocket_cfg);
    esp_websocket_register_events(*client, WEBSOCKET_EVENT_ANY, websocket_event_handler, (void *)client);

    esp_websocket_client_start(*client);

}

void ws_send(void* param)
{
    esp_websocket_client_handle_t* p_client = (esp_websocket_client_handle_t*) param;

    char data[32];
    int i = 0;
    while (i < 10) {
        if (esp_websocket_client_is_connected(*p_client)) {
            int len = sprintf(data, "hello %04d", i++);
            ESP_LOGI(TAG_WEBSOCKET, "Sending %s", data);
            esp_websocket_client_send_text(*p_client, data, len, portMAX_DELAY);
        }
        vTaskDelay(3000 / portTICK_RATE_MS);
    }

    esp_websocket_client_close(*p_client, portMAX_DELAY);
    ESP_LOGI(TAG_WEBSOCKET, "Websocket Stopped");
    esp_websocket_client_destroy(*p_client);

    vTaskDelete(NULL);
}

void app_main(void)
{
    ESP_LOGI(TAG_WEBSOCKET, "[APP] Startup..");
    ESP_LOGI(TAG_WEBSOCKET, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG_WEBSOCKET, "[APP] IDF version: %s", esp_get_idf_version());

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());

    //cria um cliente 
    static esp_websocket_client_handle_t client;

    websocket_app_start(&client);
    xTaskCreate(ws_send, "ws_send", 2048, (void *)&client, 15, NULL);
}
