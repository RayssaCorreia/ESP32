#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main(void)
{
    char c = 0;
    char *str = (char *)malloc(100); //alocando

    // zerando conteudo de str
    memset(str, 0, 100);

    printf("Digite: ");

    // Enquanto não receber um "enter"
    while (c != '\n')
    {
        // Recebe dado pela serial
        c = getchar();

        // Caso a tecla seja "backspace"
        if (c == 0x08)
        {
            c = '\0';
            // Apaga o último caracter da string
            str[((strlen(str) - 1) > 0) ? strlen(str) - 1 : 0] = c;

            // Apaga o conteúdo da linha no terminal
            printf("\x1b[2K");

            // Imprime conteúdo de str no terminal
            printf("\rDigite: %.*s", strlen(str), str);
        }
        // Caso seja caracter válido
        else if ((c >= 0x20) && (c <= 0x7e))
        {
            // Insere caracter na ultima posição de str
            str[strlen(str)] = c;

            // Apaga o conteúdo da linha no terminal
            printf("\x1b[2K");

            // Imprime conteúdo de str no terminal
            printf("\rDigite: %.*s", strlen(str), str);
        }

        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    // Apaga o conteúdo da linha no terminal
    printf("\x1b[2K");

    printf("\rvc digitou: %.*s\nFIM\n", strlen(str), str);

    if (str)
        free(str); // Desalocando memória //
}
