#include <stdio.h>
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include "mdns.h"
#include "protocol_examples_common.h"
#include "driver/gpio.h"

static const char *TAG = "MDNS_EXAMPLE";

//Altere o nome do dispositivo
#define NOME    "rayssa"
#define MDNS_NAME  NOME "_esp32"
#define INSTANCE_NAME NOME "esp32_thing"

static esp_err_t on_default_url(httpd_req_t *req)
{
  ESP_LOGI(TAG, "URL: %s", req->uri);
  httpd_resp_sendstr(req, "hello world");
  return ESP_OK;
}

static void init_server()
{
  httpd_handle_t server = NULL;
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();

  //definindo um servidor HTTP
  ESP_ERROR_CHECK(httpd_start(&server, &config));

  httpd_uri_t default_url = {
      .uri = "/",
      .method = HTTP_GET,
      .handler = on_default_url};
  httpd_register_uri_handler(server, &default_url);
}

void start_mdns_service()
{
  //setando- startando
  ESP_ERROR_CHECK(mdns_init());
  ESP_ERROR_CHECK(mdns_hostname_set(MDNS_NAME));
  ESP_LOGI(TAG, "mdns name: %s", MDNS_NAME);
  ESP_ERROR_CHECK(mdns_instance_name_set(INSTANCE_NAME));
}

void app_main(void)
{
    //cabeçalho  
    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    //conexão com a internet
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());
    
    start_mdns_service();
    init_server();
}