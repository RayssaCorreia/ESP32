#include <stdio.h>
#include "linked_list.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main()
{
    struct node* element = NULL;

    //colocando os números de maloc
    insertFirst(1);
    insertFirst(3);
    insertFirst(6);
    insertFirst(13);

    printList();

    //deletar os links(nós) e retorna o elemento
    element = deleteNode(6);
    if(element)
        free(element);
    
    printList();

    insertFirst(15);
    printList();

    while(1){
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}