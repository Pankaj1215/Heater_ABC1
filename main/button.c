/*
 *  Copyright 2020. Lucidtron Philippines. All Rights Reserved. Permission
 *  to use, copy, modify, and distribute the software and its documentation
 *  for education, research and non-for-profit purposes. without fee and without
 *  signed licensing agreement, is hereby granted, provided that the above
 *  copyright notice, this paragraph and the following two paragraphs appear
 *  in all copies, modification, and distributions. Contact The offfice of
 *  Lucidtron Philippines Inc. Unit D 2nd Floor GMV-Winsouth 2 104 
 *  East Science Avenue, Laguna Technopark, Biñan City, Laguna 4024.
 *  lucidtron@lucidtron.com (049) 302 7001 for commercial and licensing 
 *  opportunities.
 *
 *  IN NO EVENT SHALL LUCIDTRON BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
 *  SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, 
 *  ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF 
 *  LUCIDTRON HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  LUCIDTRON SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT 
 *  LIMITED TO. THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A 
 *  PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY,
 *  PROVIDED HEREUNDER IS PROVIDED "AS IS". LUCIDTRON HAS NO OBLIGATION TO PROVIDE
 *  MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENS, OR MODIFICATIONS.
 *
 */

#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "driver/gpio.h"
#include "sdkconfig.h"

#include "button.h"


typedef struct {
    xQueueHandle evt_queue;
    button_cb_t up_cb;
    button_cb_t down_cb;
    button_cb_t power_back_cb;
    button_cb_t timer_forward_cb;
} button_t;


void button_task(void *param);
static void button_isr_handler(void* param);


static char *TAG = "button";
static button_t *button;


esp_err_t button_init(void) {
    ESP_LOGD(TAG, "button_init");
    // allocate memory to button struct
    if (button == NULL) {
        button = malloc(sizeof(button_t));
    } else {
        ESP_LOGD(TAG, "already initialized");
        return ESP_OK;
    }

    if (button == NULL) {
        ESP_LOGD(TAG, "no mem");
        return ESP_ERR_NO_MEM;
    }
  

    // initialize button struct
    button->evt_queue = NULL;
    button->up_cb = NULL;
    button->down_cb = NULL;
    button->power_back_cb = NULL;
    button->timer_forward_cb = NULL;

    // initialize GPIO
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_ANYEDGE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << BUTTON_UP_GPIO_NUM) | (1ULL << BUTTON_DOWN_GPIO_NUM) | (1ULL << BUTTON_POWER_BACK_GPIO_NUM) | (1ULL << BUTTON_TIMER_FORWARD_GPIO_NUM),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE,
    };
    gpio_config(&io_conf);

    button->evt_queue = xQueueCreate(10, sizeof(uint32_t));
    xTaskCreate(button_task, "button_task", 4096, NULL, 10, NULL);

    gpio_install_isr_service(0);
    // hook isr handler for specific gpio pin
    gpio_isr_handler_add(BUTTON_UP_GPIO_NUM, button_isr_handler, (void*) BUTTON_UP_GPIO_NUM);
    gpio_isr_handler_add(BUTTON_DOWN_GPIO_NUM, button_isr_handler, (void*) BUTTON_DOWN_GPIO_NUM);
    gpio_isr_handler_add(BUTTON_POWER_BACK_GPIO_NUM, button_isr_handler, (void*) BUTTON_POWER_BACK_GPIO_NUM);
    gpio_isr_handler_add(BUTTON_TIMER_FORWARD_GPIO_NUM, button_isr_handler, (void*) BUTTON_TIMER_FORWARD_GPIO_NUM);

    return ESP_OK;
}

esp_err_t button_up_set_cb(button_cb_t cb) {
    ESP_LOGD(TAG, "button_up_set_cb");
    if (button == NULL) {
        ESP_LOGD(TAG, "not initialized");
        return ESP_FAIL;
    }

    button->up_cb = cb;

    return ESP_OK;
}

esp_err_t button_down_set_cb(button_cb_t cb) {
    ESP_LOGD(TAG, "button_up_set_cb");
    if (button == NULL) {
        ESP_LOGD(TAG, "button_down_set_cb");
        return ESP_FAIL;
    }

    button->down_cb = cb;

    return ESP_OK;
}

esp_err_t button_power_back_set_cb(button_cb_t cb) {
    ESP_LOGD(TAG, "button_power_back_set_cb");
    if (button == NULL) {
        ESP_LOGD(TAG, "not initialized");
        return ESP_FAIL;
    }

    button->power_back_cb = cb;

    return ESP_OK;
}

esp_err_t button_timer_forward_set_cb(button_cb_t cb) {
    ESP_LOGD(TAG, "button_timer_forward_set_cb");
    if (button == NULL) {
        ESP_LOGD(TAG, "not initialized");
        return ESP_FAIL;
    }

    button->timer_forward_cb = cb;

    return ESP_OK;
}

int button_up_get_level(void) {
    return gpio_get_level(BUTTON_UP_GPIO_NUM);
}

int button_down_get_level(void) {
    return gpio_get_level(BUTTON_DOWN_GPIO_NUM);
}

int button_power_back_get_level(void) {
    return gpio_get_level(BUTTON_POWER_BACK_GPIO_NUM);
}

int button_timer_forward_get_level(void) {
    return gpio_get_level(BUTTON_TIMER_FORWARD_GPIO_NUM);
}


void button_task(void *param)
{
    ESP_LOGD(TAG, "task started");

    uint32_t io_num;
    int io_level;

    while(1)
    {
        if(xQueueReceive(button->evt_queue, &io_num, portMAX_DELAY))
        {
            io_level = gpio_get_level(io_num);
            ESP_LOGV(TAG, "io_num=%d level=%d", io_num, io_level);

            switch (io_num) {
            case BUTTON_UP_GPIO_NUM:
                ESP_LOGD(TAG, "UP %d", io_level);
                if (button->up_cb != NULL)
                    button->up_cb(io_level);
                break;
            case BUTTON_DOWN_GPIO_NUM:
                ESP_LOGD(TAG, "DOWN %d", io_level);
                if (button->down_cb != NULL)
                    button->down_cb(io_level);
                break;
            case BUTTON_POWER_BACK_GPIO_NUM:
                ESP_LOGD(TAG, "POWER/BACK %d", io_level);
                if (button->power_back_cb != NULL)
                    button->power_back_cb(io_level);
                break;
            case BUTTON_TIMER_FORWARD_GPIO_NUM:
                ESP_LOGD(TAG, "TIMER/FORWARD %d", io_level);
                if (button->timer_forward_cb != NULL)
                    button->timer_forward_cb(io_level);
                break;
            }
        }
    }

    vTaskDelete(NULL);
}

static void IRAM_ATTR button_isr_handler(void* param)
{
    uint32_t io_num = (uint32_t) param;
    xQueueSendFromISR(button->evt_queue, &io_num, NULL);
}

