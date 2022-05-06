#include <stdio.h>
#include "esp_log.h"

const char *TAG ="hello.c";

void app_main(void) {
    printf("Hello world!\n");
    // 2 parametros teg, e o que ele vai passa 
    ESP_LOGI("TAG","hello world!");
    ESP_LOGW("TAG","Configuração!");
    ESP_LOGE("TAG","ERROR!");
}