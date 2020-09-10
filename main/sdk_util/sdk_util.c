#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

#include "sdk_util.h"
#include "common.h"

#include "stdlib.h"

void* create_driver(int size)
{
    return malloc(size);
}

int create_thread(void (*thread)(void* param), char* label)
{
    xTaskCreate(thread, label, 4096, NULL, 5, NULL);
    return SUCCESS;
}
int create_thread_with_stackvalue(void (*thread)(void* param), char* label, int stackvalue)
{
    xTaskCreate(thread, label, stackvalue, NULL, 5, NULL);
    return SUCCESS;
}

int delay_sec(int sec)
{
    int i;

    for(i = 0; i<sec; i++)
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    return SUCCESS;
}

int delay_milli(int milli)
{
    vTaskDelay(milli / portTICK_PERIOD_MS);
    return SUCCESS;
}

void reset_device(void)
{
    esp_restart();
}
