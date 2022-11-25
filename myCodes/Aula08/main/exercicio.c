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

#include "driver/uart.h"
#include "string.h"
#include "driver/gpio.h"

#include <time.h>
#include "esp_sntp.h"

#define SERVER_URI  "ws://172.16.107.13"
#define SERVER_PORT 8000
#define SERVER_PATH "/websocket"

#define TXD_PIN (GPIO_NUM_17)
#define RXD_PIN (GPIO_NUM_16)

#define UART_NUM UART_NUM_2

#define PURPLE "\x1b[35m"
#define LIGHT_BLUE "\x1b[36m"
#define RESET "\x1b[0m"

#define NAME "Rayssa"

#define TAGTIME "NTP_TIME"

char texto[100] = {0};
int mensagem = 0;

static const char *TAG = "WEBSOCKET";

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
    case WEBSOCKET_EVENT_CONNECTED: // conectou
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_CONNECTED");
        break;
    case WEBSOCKET_EVENT_DISCONNECTED: // desconectou
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_DISCONNECTED");
        break;
    case WEBSOCKET_EVENT_DATA: // enviando dado
        // ESP_LOGI(TAG, "WEBSOCKET_EVENT_DATA");
        // ESP_LOGI(TAG, "Received opcode=%s", opCodeToString(data->op_code));
        if (data->op_code == 0x08) {
            ESP_LOGW(TAGTIME, "Received closed message with code=%d", 256*data->data_ptr[0] + data->data_ptr[1]);
        } else {
            if(data->data_len > 0)
                ESP_LOGW(TAG, "%.*s\n", data->data_len, (char *)data->data_ptr);
                memset(data->data_ptr, 0, strlen(data->data_ptr));
        }
        // ESP_LOGW(TAG, "Total payload length=%d, data_len=%d, current payload offset=%d\r\n", data->payload_len, data->data_len, data->payload_offset);

        break;
    case WEBSOCKET_EVENT_ERROR: // erro
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_ERROR");
        break;
    }
}

static void websocket_app_start(esp_websocket_client_handle_t *client)
{
    esp_websocket_client_config_t websocket_cfg = {};

    websocket_cfg.uri = SERVER_URI;
    websocket_cfg.port = SERVER_PORT;
    websocket_cfg.path = SERVER_PATH;

    ESP_LOGI(TAG, "Connecting to %s...", websocket_cfg.uri);

    *client = esp_websocket_client_init(&websocket_cfg);
    esp_websocket_register_events(*client, WEBSOCKET_EVENT_ANY, websocket_event_handler, (void *)client);

    esp_websocket_client_start(*client);

}

void ws_send(void* param)
{
    esp_websocket_client_handle_t* p_client = (esp_websocket_client_handle_t*) param;

    char data[110];
    while (1) {
        if(mensagem == 1) {
            if (esp_websocket_client_is_connected(*p_client)) { // se conectado ao servidor
                int len = sprintf(data, "%s: %s", NAME, texto);
                // ESP_LOGI(TAG, "Sending %s\n\n", data);
                esp_websocket_client_send_text(*p_client, data, len, portMAX_DELAY);
            }
            mensagem = 0;
        }
        vTaskDelay(3000 / portTICK_RATE_MS);
    }

    esp_websocket_client_close(*p_client, portMAX_DELAY);
    ESP_LOGI(TAG, "Websocket Stopped");
    esp_websocket_client_destroy(*p_client);

    vTaskDelete(NULL);
}
 
// teclado
static void teclado(void *param)
{
    char c = 0;

    // Aloca na na memória
    char *str = (char *)malloc(100);

    while (1)
    {
        c = 0;

        // Zerando conteudo de str
        memset(str, 0, 100);

        // printf("Digite: ");

        // Enquanto não receber um "enter"
        while (c != '\n')
        {
            // Recebe dado pela serial
            c = getchar();

            // Caso a tecla seja "backspace"
            if (c == 0x08)
            {
                c = '\0';
                // Apaga o último caracter da string
                str[((strlen(str) - 1) > 0) ? strlen(str) - 1 : 0] = c;

                // Apaga o conteúdo da linha no terminal
                printf("\x1b[2K");

                // Imprime conteúdo de str no terminal
                printf("\r%.*s", strlen(str), str);
            }
            // Caso seja caracter válido
            else if ((c >= 0x20) && (c <= 0x7e))
            {
                // Insere caracter na ultima posição de str
                str[strlen(str)] = c;

                // Apaga o conteúdo da linha no terminal
                printf("\x1b[2K");

                // Imprime conteúdo de str no terminal
                printf("\r%.*s", strlen(str), str);
            }

            vTaskDelay(100 / portTICK_PERIOD_MS);
        }

        if(mensagem == 0) {

            for (int i = 0; i < strlen(str); i++)
            {
                texto[i] = str[i];
            }

            texto[strlen(str)] = "/0";            
            // memcpy (texto, str, strlen(str));
            
            printf("\n");
            ESP_LOGE(TAG, "%s: %.*s\n", NAME, strlen(str), str);

            mensagem = 1;
        }
    }
}

void print_time(long time, const char *message)
{
    struct tm *timeinfo = localtime(&time);
    char buffer[50];
    
    strftime(buffer, sizeof(buffer), "%c", timeinfo);
    // ESP_LOGI(TAGTIME, "message: %s: %s", message, buffer);
}

void on_got_time(struct timeval *tv)
{
    print_time(tv->tv_sec, "time at callback");
}

void app_main(void)
{
    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());
    static esp_websocket_client_handle_t client; // parametro
    websocket_app_start(&client);

    sntp_set_sync_mode(SNTP_SYNC_MODE_IMMED); // metodo de sincronizacao
    sntp_setservername(0, "pool.ntp.org");
    sntp_init(); // inicia a funcao

    setenv("TZ", "<-03>3", 1);
    tzset();
    
    sntp_set_time_sync_notification_cb(on_got_time);

    xTaskCreate(ws_send, "ws_send", 2048, (void *)&client, 15, NULL);
    xTaskCreate(teclado, "teclado", 1024 * 2, NULL, configMAX_PRIORITIES - 2, NULL);
}