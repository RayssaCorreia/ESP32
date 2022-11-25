#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "esp_sntp.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "protocol_examples_common.h"
#include "esp_log.h"

#define TAG "NTP_TIME"

void print_time(long time, const char *message)
{
    struct tm *timeinfo = localtime(&time);
    char buffer[50];
    
    strftime(buffer, sizeof(buffer), "%c", timeinfo);
    ESP_LOGI(TAG, "message: %s: %s", message, buffer);
}

void on_got_time(struct timeval *tv)
{
    printf("secs %ld\n", tv->tv_sec);
    print_time(tv->tv_sec, "time at callback");
    
    for (int i = 0; i < 5; i++)
    {
        time_t now = 0;
        time(&now);
        print_time(now, "in loop");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void app_main(void)
{
    time_t now = 0;
    time(&now);
    print_time(now, "Beginning of application");

    //Receita de bolo
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in
     * menuconfig. Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */

    //Conexão com a internet 
    ESP_ERROR_CHECK(example_connect());

    //qual o metado 
    sntp_set_sync_mode(SNTP_SYNC_MODE_IMMED);
    //Servidor padrão
    sntp_setservername(0, "pool.ntp.org");
    sntp_init();

    setenv("TZ", "<-03>3", 1);
    tzset();
    
    sntp_set_time_sync_notification_cb(on_got_time);
}