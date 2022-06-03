#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "freertos/queue.h"

//Criar uma queue(fila), global
xQueueHandle queue;

//funcao para mandar 
void sender(void *params)
{
    int count = 0;
    while (true)
    {
        //envia atravez de um contador 
        count++;
        if (xQueueSend(queue, &count, 10) != pdTRUE)
        {
            printf("Queue FULL\n");
        }
        // metodo de bloqueio - mandando elementos para queue(fila) 
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

//funcao para receber
void receiver(void *params)
{
    while (true)
    {
        int rxInt;
        //recebendo os dados do contador 
        if (xQueueReceive(queue, &rxInt, 0) == pdTRUE)
        {
            //mostrando o que recebeu
            printf("received %d\n", rxInt);
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{// [1] fila com [3] elemento do tipo [inteiro] 
    queue = xQueueCreate(3, sizeof(int));

    // criando duas tarefas
    xTaskCreate(&sender, "send", 2048, NULL, 2, NULL);
    xTaskCreate(&receiver, "receive", 2048, NULL, 1, NULL);
}