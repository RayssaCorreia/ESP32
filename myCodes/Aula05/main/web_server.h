#ifndef WEB_SERVER_H_
#define WEB_SERVER_H_
#include "freertos/queue.h"

typedef struct {
	int8_t lenght;
	char message[128];
} post_messages_t;

void start_webserver(void);


extern QueueHandle_t xQueueHttp;
#endif