/* GPIO Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

#define PIN_SWITCH     15
#define ESP_INTR_FLAG_DEFAULT 0

static xQueueHandle gpio_evt_queue = NULL;

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

static void gpio_task_example(void* arg)
{
    uint32_t io_num;
    int cnt = 0;
    int level = 0;
    for(;;) {
        if(xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
            level = gpio_get_level(io_num);
            printf("intr #%d GPIO[%d] %s\n", cnt++, io_num, level?"pressionado":"solto");
        }
    }
}

void app_main(void)
{

    gpio_config_t in_conf = {
        .intr_type = GPIO_INTR_POSEDGE,         //interrupt of rising edge
        .pin_bit_mask = (1ULL<<PIN_SWITCH),     //bit mask of the pins, use GPIO15 here
        .mode = GPIO_MODE_INPUT,                //set as input mode
        .pull_up_en = 0,                        //disable pull-up mode
        .pull_down_en = 1,                      //enable pull-down mode
    };

    gpio_config(&in_conf);

    //create a queue to handle gpio event from isr
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    //start gpio task
    xTaskCreate(gpio_task_example, "gpio_task_example", 2048, NULL, 10, NULL);

    //install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(PIN_SWITCH, gpio_isr_handler, (void*) PIN_SWITCH);

    printf("Minimum free heap size: %d bytes\n", esp_get_minimum_free_heap_size());

}