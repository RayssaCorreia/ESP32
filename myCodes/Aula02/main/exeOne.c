#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define PIN_BUTTON 15
#define PIN_LED 23

void app_main()
{
    gpio_pad_select_gpio(PIN_LED);                 // define o pino
    gpio_set_direction(PIN_LED, GPIO_MODE_OUTPUT); // modo do pino -saida

    gpio_pad_select_gpio(PIN_BUTTON);                // define o pino
    gpio_set_direction(PIN_BUTTON, GPIO_MODE_INPUT); // modo do pino -entrada

    gpio_pulldown_en(PIN_BUTTON);
    gpio_pullup_dis(PIN_BUTTON);

    while (1)
    {

        int qnivelPin = gpio_get_level(PIN_BUTTON);

        if (qnivelPin == 1) //precionado em 2hz
        {
            gpio_set_level(PIN_LED, 1); //ligar led
            vTaskDelay(pdMS_TO_TICKS(250)); // um tempo de espera
            gpio_set_level(PIN_LED, 0);
            vTaskDelay(pdMS_TO_TICKS(250));
        }

        else if
            (qnivelPin == 0) // solto em 10hz
            {
                gpio_set_level(PIN_LED, 1);
                vTaskDelay(pdMS_TO_TICKS(50));
                gpio_set_level(PIN_LED, 0);
                vTaskDelay(pdMS_TO_TICKS(50));
            }
    }
}