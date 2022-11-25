#include <stdio.h>
#include "protocol_examples_common.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include "esp_wifi.h"
#include "mdns.h"
#include "cJSON.h"
#include "freertos/event_groups.h"
#include "linked_list.h"

static const char *TAG = "serverRandom.c";
static httpd_handle_t server = NULL;

//Altere o nome do dispositivo
#define NOME    "rayssa"
#define MDNS_NAME  NOME "-esp32"
#define INSTANCE_NAME NOME "_esp32_thing"

static esp_err_t on_default_url(httpd_req_t *req)
{
    ESP_LOGI(TAG, "URL: %s", req->uri);
    httpd_resp_sendstr(req, "hello world");
    return ESP_OK;
}

/********************Web Socket *******************/
#define WS_MAX_SIZE 4096

esp_err_t send_ws_message(char *message)
{
    esp_err_t ret = ESP_OK;
    struct node* first = getFirst();
                        //tamanho da lista lincada 
    for( int i = 0; i < length(); i++) 
    {
        httpd_ws_frame_t ws_message = {.final = true,
                                    .fragmented = false,
                                    .len = strlen(message),
                                    .payload = (uint8_t *)message,
                                    .type = HTTPD_WS_TYPE_TEXT};
             // ennvia dados    
       ret = httpd_ws_send_frame_async(server, first->socket, &ws_message);
       first = first->next;
    }

    return ret;
}

static esp_err_t on_web_socket_url(httpd_req_t *req)
{    
    if (req->method == HTTP_GET)
        return ESP_OK;

    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    ws_pkt.payload = malloc(WS_MAX_SIZE);
    httpd_ws_recv_frame(req, &ws_pkt, WS_MAX_SIZE); 
    printf("ws payload: %.*s\n", ws_pkt.len, ws_pkt.payload); // o que recebo o que printou no termenal 
    
    free(ws_pkt.payload);
    return ESP_OK;
}

/*******************************************/


esp_err_t open_socket_handler(httpd_handle_t server, int sockfd) {
    //qual
    ESP_LOGI(TAG, "client %d connected", sockfd);
    //add a lista 
    insertFirst(sockfd);
    return ESP_OK;
}

void closed_socket_handler(httpd_handle_t server, int sockfd) {
    struct node* socket = deleteNode(sockfd);
    if(socket)
        free(socket);
    ESP_LOGI(TAG, "client %d disconnected", sockfd);
}

static void init_server()
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.close_fn = closed_socket_handler;
    config.open_fn = open_socket_handler;

    ESP_ERROR_CHECK(httpd_start(&server, &config));

    httpd_uri_t default_url = {
        .uri = "/", .method = HTTP_GET, .handler = on_default_url};
    httpd_register_uri_handler(server, &default_url);

    httpd_uri_t web_socket_url = {.uri = "/ws",
                                  .method = HTTP_GET,
                                  .handler = on_web_socket_url,
                                  .is_websocket = true};
    httpd_register_uri_handler(server, &web_socket_url);
}

void start_mdns_service()
{
    mdns_init();
    mdns_hostname_set(MDNS_NAME);
    mdns_instance_name_set(INSTANCE_NAME);
}

void app_main(void)
{
    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ESP_ERROR_CHECK(example_connect());
    
    start_mdns_service();
    init_server();

    while(1)
    {
        cJSON *object = cJSON_CreateObject();
        cJSON_AddStringToObject(object, "topic", "random_number"); // criando 

                                                   // número aleatorios de 0 a 99 + 1, aleatorios 0 a 100 
        cJSON_AddNumberToObject(object, "payload", esp_random()%99 + 1);
    
        char *message = cJSON_Print(object);
        printf("message: %s\n", message);
        send_ws_message(message);

cJSON_Delete(object);
        free(message);

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}