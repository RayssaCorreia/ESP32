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

/*
*
*   https://github.com/zonmen/IndoorSolution-esp32/issues/1#issuecomment-890380384
*
*/

// tipos de bits
static EventGroupHandle_t wifi_events;
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1
// tipos

//Variaveis
#define MAX_RETRY 10
static int retry_cnt = 0;
static const char *TAG = "wifi_app";
static void request_page(void *);
static esp_err_t handle_http_event(esp_http_client_event_t *);

//prototipo de função
static void handle_wifi_connection(void *, esp_event_base_t,
int32_t, void *);

#define WIFI_SSID "dilson"
#define WIFI_PASS "Cl@udionor"

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

// receber todos os eventos (estatus do wi-fi)
static void handle_wifi_connection(void *arg, esp_event_base_t event_base, 
                                int32_t event_id, void *event_data)
{
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
static void request_page(void *arg)
{
    esp_http_client_config_t config = 
    {
        .url = "https://www.google.com/",
        .event_handler = handle_http_event,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (esp_http_client_perform(client) != ESP_OK)
    {
        ESP_LOGE(TAG, "http request failed");
    }
    esp_http_client_cleanup(client);
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
    //Chamando uma função
    init_wifi();
}