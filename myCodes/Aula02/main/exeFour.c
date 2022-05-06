#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"

void app_main(void) {
    
    while(1) {
        int sensor = hall_sensor_read(); // ler o valor do sensor
        
        printf("Leitura do Sensor: %d\n", sensor);
        vTaskDelay(pdMS_TO_TICKS(500)); // delay
    }
}