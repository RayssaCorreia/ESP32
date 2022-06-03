#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void task1(void *param)
{
    while (1)

    // ESTRUTURA PARA MILISSEGUNDOS //
    {   
        //trabalha em "ticks"
        vTaskDelay(pdMS_TO_TICKS(100));       // delay de 100 milissegundos
        vTaskDelay(100 / portTICK_PERIOD_MS); // delay de 100 milissegundos
    }
}

void app_main(void)
{
    xTaskCreate(&task1, "task1", 2048, NULL, 0, NULL);
}