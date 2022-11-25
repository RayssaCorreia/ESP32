#include <stdio.h>
#include <string.h>
#include "cJSON.h"
#include "esp_log.h"
#include "esp_system.h"

const char *TAG = "simpleJSON.c";

void app_main()
{   
    //Obtendo a quantidade de memória dinâmica no início do programa
    uint32_t initialHeap = esp_get_free_heap_size();

    //criando um objeto json
    cJSON *object_json = cJSON_CreateObject();

    //adicionando chaves/valores ao objeto json
    cJSON_AddStringToObject(object_json, "nome","dilson");
    cJSON_AddStringToObject(object_json, "sobrenome","ito");
    cJSON_AddNumberToObject(object_json, "idade", 35);
    
    //criando um novo objeto json
    cJSON *cad_profissional = cJSON_CreateObject();

    //adicionando chaves/valores ao novo objeto json
    cJSON_AddStringToObject(cad_profissional, "Empresa", "Hausenn");
    cJSON_AddStringToObject(cad_profissional, "Cargo", "desenvolvedor");

    //adicionando o novo objeto no object_json
    cJSON_AddItemToObject(object_json, "cad_profissional", cad_profissional);

    //criando um array
    cJSON *tamanhos = cJSON_CreateArray();

    //criando números como itens json
    cJSON *camisa = cJSON_CreateNumber(2);
    cJSON *calca = cJSON_CreateNumber(38);
    cJSON *calcado = cJSON_CreateNumber(39);

    //adicionando os itens numeros ao array
    cJSON_AddItemToArray(tamanhos, camisa);
    cJSON_AddItemToArray(tamanhos, calca);
    cJSON_AddItemToArray(tamanhos, calcado);

    //adicionando array ao object_json
    cJSON_AddItemToObject(object_json, "tamanhos", tamanhos);

    //adicionando booleano ao object_json
    cJSON_AddBoolToObject(object_json, "professor_padolabs", true);
    
    //criando um objeto json nulo
    cJSON *plano_saude = cJSON_CreateNull();
    //adicionando nulo ao object_json
    cJSON_AddItemToObject(object_json, "cadastro_plano_saude", plano_saude);
                                                                                            // | mandando
    //##########################################################################################################
                                                                                            // | recebendo 
                                                                                        
    //criando uma variável para armazenar o valor de object_json em string
    char *object_string = NULL;
    object_string = cJSON_Print(object_json);
    ESP_LOGI(TAG, "conteudo json: %.*s", strlen(object_string),object_string);
    
    //parse do object_string
    cJSON *parse_json = cJSON_Parse(object_string);

    //obtendo o valor da chave "nome"
    cJSON *nome_json = cJSON_GetObjectItem(parse_json,"nome");
    ESP_LOGI(TAG, "valor de nome: %s", cJSON_GetStringValue(nome_json));
    
    //obtendo o valor da chave "sobrenome"
    cJSON *sobrenome_json = cJSON_GetObjectItem(parse_json,"sobrenome");
    ESP_LOGI(TAG, "valor de sobrenome: %s", cJSON_GetStringValue(sobrenome_json));

    //obtendo o valor da chave "idade"
    cJSON *idade_json = cJSON_GetObjectItem(parse_json,"idade");
    ESP_LOGI(TAG, "valor de idade: %f", cJSON_GetNumberValue(idade_json));

    //obtendo o objeto da chave: "cad_profissional"
    cJSON *cad_prof_json = cJSON_GetObjectItem(parse_json,"cad_profissional");

    //obtendo o valor da chave "Empresa"  
    cJSON *empresa_json = cJSON_GetObjectItem(cad_prof_json, "Empresa");
    ESP_LOGI(TAG, "valor de empresa: %s", cJSON_GetStringValue(empresa_json));

    //obtendo o valor da chave "Cargo"
    cJSON *cargo_json = cJSON_GetObjectItem(cad_prof_json, "Cargo");
    ESP_LOGI(TAG, "valor de cargo: %s", cJSON_GetStringValue(cargo_json));
    
    //obtendo o array da chave: "tamanhos"
    cJSON *tamanhos_json = cJSON_GetObjectItem(parse_json,"tamanhos");

    //obtendo o 1o valor do array
    cJSON *camisa_json = cJSON_GetArrayItem(tamanhos_json, 0);
    ESP_LOGI(TAG, "valor de camisa: %f", cJSON_GetNumberValue(camisa_json));

    //obtendo o 2o valor do array
    cJSON *calca_json = cJSON_GetArrayItem(tamanhos_json, 1);
    ESP_LOGI(TAG, "valor de calca: %f", cJSON_GetNumberValue(calca_json));

    //obtendo o 3o valor do array
    cJSON *calcado_json = cJSON_GetArrayItem(tamanhos_json, 2);
    ESP_LOGI(TAG, "valor de calcado: %f", cJSON_GetNumberValue(calcado_json));

    //obtendo o valor da chave "professor_padolabs"
    cJSON *prof_padolabs_json = cJSON_GetObjectItem(parse_json,"professor_padolabs");
    ESP_LOGI(TAG, "valor de professor_padolabs: %s", cJSON_GetNumberValue(prof_padolabs_json)?"true": "false");
    
    //obtendo objeto da chave "cadastro_plano_saude"
    cJSON *cad_plano_json = cJSON_GetObjectItem(parse_json,"cadastro_plano_saude");
    ESP_LOGI(TAG, "valor de cadastro_plano_saude: %s", cJSON_IsNull(cad_plano_json)?"null":"not null");

    //deletando o objeto parse_json
    if(parse_json)
        cJSON_Delete(parse_json);
    
    //deletando o objeto object_json
    if(object_json)
        cJSON_Delete(object_json);

    //deletando a string
    if (object_string)
        cJSON_free(object_string);

    //Obtendo a quantidade de memória dinâmica no final do programa e fazendo alguns cálculos
    uint32_t finalHeap = esp_get_free_heap_size();
    ESP_LOGI(TAG, "(%s) Heap(I:%d, F:%d, I-F:%d, M:%d)", __func__,
             initialHeap, finalHeap, 
             initialHeap - finalHeap, 
             esp_get_minimum_free_heap_size());
}