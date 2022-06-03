#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

// trazer um comunicado para o terminal
const char *TAG = "tasks.c";

// Criando um tasks
void task1(void *param)
{
    // Lup principal da tarefa (infinito)
    while (true)
    {
        // Lendo temperatura [...]
        ESP_LOGI(TAG, "core %d/line %d/%s/reading temperature/%d",
                                    //Mostrar a linha que esta no terminal
                 xPortGetCoreID(), __LINE__, __func__,
                 // Mostrar o maximo de memoria que ela utilizou.
                 uxTaskGetStackHighWaterMark(NULL));
        // [...] a cada um segundo
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    // Deletar a tasks
    vTaskDelete(NULL);
}

void task2()
{
    while (true)
    {
        // Lendo umidade
        ESP_LOGW(TAG, "core %d/line %d/%s/reading humidity/%d",
                 xPortGetCoreID(), __LINE__, __func__,
                 uxTaskGetStackHighWaterMark(NULL));
        // a cada 2 segundo
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

void app_main(void)
{
    ESP_LOGI(TAG, "core %d/line %d/%s/starting ", xPortGetCoreID(), __LINE__,
             __func__);

    // CRIANDO TASKS DEIXANDO O FreRTOS ESCOLHER O NUCLEO
    // xTaskCreate(&task1, "temperature reading", 2048, NULL, 2, NULL);
    // xTaskCreate(&task2, "humidity reading", 2048, NULL, 2, NULL);

    // CRIANDO TASKS ESPECIFICANDO O CORE (NUCLEO)
    xTaskCreatePinnedToCore(&task1, "temperature reading", 2048, NULL, 2, NULL,0);
    xTaskCreatePinnedToCore(&task2, "humidity reading", 2048, NULL, 2, NULL, APP_CPU_NUM);
}