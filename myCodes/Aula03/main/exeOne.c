#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "freertos/queue.h"

#define PIN_BUTTON 15
#define PIN_LED 23

xQueueHandle queue;
int qnivelPin = 0;



void app_main()
{

    gpio_pad_select_gpio(PIN_LED);                 // define o pino
    gpio_set_direction(PIN_LED, GPIO_MODE_OUTPUT); // modo do pino -saida

    gpio_pad_select_gpio(PIN_BUTTON);                // define o pino
    gpio_set_direction(PIN_BUTTON, GPIO_MODE_INPUT); // modo do pino -entrada

    gpio_pulldown_en(PIN_BUTTON);
    gpio_pullup_dis(PIN_BUTTON);

    

   // xTaskCreate(&Button, "LED 10hz", 2048, NULL, 1, NULL);
  //  xTaskCreate(&LED, "LED 2hz", 2048, NULL, 2, NULL);
}