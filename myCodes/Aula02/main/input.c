#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define PIN_SWITCH 15
#define PIN_LED 23

const char *TAG = "input.c";

void app_main(void)
{
    printf("Switch and LED\n");
    ESP_LOGI(TAG, "Starting!");

    gpio_pad_select_gpio(PIN_LED);
    gpio_set_direction(PIN_LED, GPIO_MODE_OUTPUT);

    gpio_pad_select_gpio(PIN_SWITCH);
    gpio_set_direction(PIN_SWITCH, GPIO_MODE_INPUT);

    gpio_pulldown_en(PIN_SWITCH);
    gpio_pullup_dis(PIN_SWITCH);

    while (true)
    {
        int level = gpio_get_level(PIN_SWITCH);
        gpio_set_level(PIN_LED, level);
        vTaskDelay(1);
    }
}