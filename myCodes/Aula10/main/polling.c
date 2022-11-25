#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define PIN_SWITCH 15
const char *TAG = "polling.c";

void app_main(void)
{
    //modo de configuração do botão 
    #if 1

        gpio_pad_select_gpio(PIN_SWITCH);
        gpio_set_direction(PIN_SWITCH, GPIO_MODE_INPUT);
        gpio_pulldown_en(PIN_SWITCH);
        gpio_pullup_dis(PIN_SWITCH);
    
    #else 

        gpio_config_t io_conf = {
            .intr_type = GPIO_INTR_DISABLE,
            .pin_bit_mask = (1ULL<<PIN_SWITCH),
            .mode = GPIO_MODE_INPUT,
            .pull_up_en = 0,
            .pull_down_en = 1
        };
        gpio_config(&io_conf);

    #endif


    while (true)
    {
        ESP_LOGI(TAG, "Botão %d: %s", PIN_SWITCH, gpio_get_level(PIN_SWITCH)?"pressionado":"solto");
        vTaskDelay(100);
    }
}