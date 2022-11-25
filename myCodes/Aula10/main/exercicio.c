#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "esp_http_client.h"
#include "button.h"
#include "cJSON.h"


static EventGroupHandle_t wifi_events;
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1
// tipos

#define WIFI_SSID "arielly"
#define WIFI_PASS "12345678"

#define PIN_SWITCH 15

const static char *TAG_BUTTON = "debounce.c";

//Variaveis
#define MAX_RETRY 10
static int retry_cnt = 0;
static const char *TAG = "wifi_app";
static void request_page(void *);
static esp_err_t handle_http_event(esp_http_client_event_t *);

//prototipo de função
static void handle_wifi_connection(void *, esp_event_base_t,
int32_t, void *);


void button_handler (button_queue_t param)
{
    
    static uint32_t cnt = 0;
    char tag[30] = {0};
    sprintf(tag, "button_handler %d", cnt++);

    switch(param.buttonState)
    {
        case BTN_PRESSED:
        ESP_LOGI(tag, "botão %d %s", param.buttonPin, "pressionado");
        break;

        case BTN_RELEASED:
        ESP_LOGI(tag, "botão %d %s", param.buttonPin, "solto");
        break;

        case BTN_HOLD:
        ESP_LOGI(tag, "botão %d %s", param.buttonPin, "mantido pressionado");
        break;
    }
    
}


static void init_wifi(void)
{
    if (nvs_flash_init() != ESP_OK)
    {
        nvs_flash_erase();
        nvs_flash_init();
    }

    wifi_events = xEventGroupCreate();
    esp_event_loop_create_default();

    //Gerenciamento de eventos - ter estrutura para conseguir trabalhar com elas
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &handle_wifi_connection, NULL);
    // Qualquer evento wi-fi: notificar essa função
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &handle_wifi_connection, NULL);
    // Qualquer evento IP: notificar essa função

    // Configuração de WI-FI STA
    wifi_config_t wifi_config = {
        .sta = {
            // conecta nesse SSID
            .ssid = WIFI_SSID,
            //QUAl a senha da rede
            .password = WIFI_PASS,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {
                .capable = true,
                .required = false},
        },
    };

    esp_netif_init();
    esp_netif_create_default_wifi_sta();
    //Configuração padrão 
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);

    //Inicializou o wi-fi (startou)
    esp_wifi_start();

    //espera algo acontecer 
    EventBits_t bits = xEventGroupWaitBits(wifi_events, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE, portMAX_DELAY);

    //se conectou
    if (bits & WIFI_CONNECTED_BIT)
    {
        xTaskCreate(request_page, "http_req", 5 *
        configMINIMAL_STACK_SIZE, NULL, 5, NULL);
    }
    else
    {
        ESP_LOGE(TAG, "failed");
    }
}

static void handle_wifi_connection(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data){
    //Quando estartou
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        //conectar
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (retry_cnt++ < MAX_RETRY)
        {
            esp_wifi_connect();
            ESP_LOGI(TAG, "wifi connect retry: %d", retry_cnt);
        }
        else
        {
            xEventGroupSetBits(wifi_events, WIFI_FAIL_BIT);
        }
    }
    //consegui ganhar um ip
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        //printa ip recebido 
        ESP_LOGI(TAG, "ip: %d.%d.%d.%d", IP2STR(&event->ip_info.ip));
        retry_cnt = 0;
        xEventGroupSetBits(wifi_events, WIFI_CONNECTED_BIT);
    }
}

//reqisite do tipo GET
// static void request_page(void *arg)
// {
//     esp_http_client_config_t config = 
//     {
//         .url = "https://www.google.com/",
//         .event_handler = handle_http_event,
//     };
//     esp_http_client_handle_t client = esp_http_client_init(&config);
//     if (esp_http_client_perform(client) != ESP_OK)
//     {
//         ESP_LOGE(TAG, "http request failed");
//     }
//     esp_http_client_cleanup(client);
//     vTaskDelete(NULL);
// }

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


static esp_err_t handle_http_event(esp_http_client_event_t *http_event)
{
    switch (http_event->event_id)
    {
        case HTTP_EVENT_ON_DATA:
            printf("%.*s\n", http_event->data_len, 
                    (char *)http_event->data);
        break;

        default:
        break;
    }
    return ESP_OK;
}

void app_main(void)

{
      button_init_t button = {
        .buttonEventHandler = button_handler,
        .pin_bit_mask = (1ULL<<PIN_SWITCH),
        .pull_up_en = 0,
        .pull_down_en = 1
    };

    buttonInit(&button);

    ESP_LOGI(TAG_BUTTON, "started");

    //Chamando uma função
    init_wifi();
}