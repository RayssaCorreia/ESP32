#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define PIN_LED 23

const char *TAG = "output.c";

void app_main(void)
{
    ESP_LOGI(TAG, "Starting!");

    gpio_pad_select_gpio(PIN_LED);
    gpio_set_direction(PIN_LED, GPIO_MODE_OUTPUT);

    bool isOn = 0;
    while (1)
    {
        isOn = !isOn;
        gpio_set_level(PIN_LED, isOn);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}