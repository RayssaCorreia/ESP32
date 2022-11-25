#include <stdio.h>
#include "esp_log.h"
#include "button.h"

#define PIN_SWITCH 15

const static char *TAG = "debounce.c";

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

void app_main() 
{
    button_init_t button = {
        .buttonEventHandler = button_handler,
        .pin_bit_mask = (1ULL<<PIN_SWITCH),
        .pull_up_en = 0,
        .pull_down_en = 1
    };

    buttonInit(&button);

    ESP_LOGI(TAG, "started");
}