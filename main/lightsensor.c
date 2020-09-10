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
#include <math.h>
#include "esp_log.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "driver/i2c.h"
#include "driver/adc.h"
#include "sdkconfig.h"

#include "lightsensor.h"


#define LIGHTSENSOR_MUTEX_LOCK(lock)    do {} while (xSemaphoreTake(lock, portMAX_DELAY) != pdPASS)
#define LIGHTSENSOR_MUTEX_UNLOCK(lock)  xSemaphoreGive(lock)


static xSemaphoreHandle ls_lock = NULL;
static int rolling_ave[LDR_ROLLING_AVE_SAMPLES_CNT];


esp_err_t lightsensor_init(void) {
    esp_err_t ret = ESP_OK;

    if (ls_lock == NULL)
        ls_lock = xSemaphoreCreateMutex();

    LIGHTSENSOR_MUTEX_LOCK(ls_lock);

    // configure ADC
    ret |= adc1_config_width(LDR_ADC_WIDTH);
    ret |= adc1_config_channel_atten(LDR_ADC_IN_CHANNEL, LDR_ADC_ATTEN);

    LIGHTSENSOR_MUTEX_UNLOCK(ls_lock);

    // initialize GPIO
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = 1ULL << LDR_ADC_IN_CHANNEL_GPIO_NUM,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE,
    };
    ret |= gpio_config(&io_conf);

    return ret;
}

int lightsensor_get_val(void) {
    int sample = 0;
    int sample_prev = 0;
    int total = 0;
    int ave = 0;
    int total_rolling_ave = 0;
    int resample_cnt = 0;
    int rolling_ave_ave = 0;

    // get samples and average
    for(int i = 0; i < LDR_SAMPLES_CNT; i++) {
        resample_cnt = LDR_RESAMPLE_CNT_MAX;
        sample = adc1_get_raw(LDR_ADC_IN_CHANNEL);
        if (i) {
            // resample if the sample is higher or lower by LDR_MAX_DIFF_ADC than the previous sample
            while ((((sample - sample_prev) >= LDR_MAX_DIFF_ADC) \
                || ((sample_prev - sample) >= LDR_MAX_DIFF_ADC)) \
                && resample_cnt) {
                sample = adc1_get_raw(LDR_ADC_IN_CHANNEL);
                --resample_cnt;
            }
        }
//        printf("sample=%d\r\n", sample);
        sample_prev = sample;

        // get total
        total += sample;
    }
    ave = total / LDR_SAMPLES_CNT;
//    printf("ave=%d\r\n", ave);

    // push new average to buffer
    for (int i = LDR_ROLLING_AVE_SAMPLES_CNT - 1; i > 0; i--)
        rolling_ave[i] = rolling_ave[i - 1];
    rolling_ave[0] = ave;

    // get the total rolling averages
    for (int i = 0; i < LDR_ROLLING_AVE_SAMPLES_CNT; i++) {
        // handle unset rolling average index
        if (rolling_ave[i] == -1)
            total_rolling_ave += ave;
        else
            total_rolling_ave += rolling_ave[i];
    }

    rolling_ave_ave = total_rolling_ave / LDR_ROLLING_AVE_SAMPLES_CNT;

    return (rolling_ave_ave * 100 / LDR_ADC_MAX_VAL);
}

