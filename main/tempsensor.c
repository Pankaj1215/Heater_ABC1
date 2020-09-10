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
#include "esp_adc_cal.h"
#include "sdkconfig.h"

#include "tempsensor.h"


static float adc_to_degc(int adc_sample);


static esp_adc_cal_characteristics_t characteristics;
static int rolling_ave[ROLLING_AVE_SAMPLES_CNT];


esp_err_t tempsensor_init(void) {
    esp_err_t ret = ESP_OK;

    // configure ADC
    ret |= adc1_config_width(NTC_ADC_WIDTH);
    ret |= adc1_config_channel_atten(NTC_ADC_IN_CHANNEL, NTC_ADC_ATTEN);
    esp_adc_cal_get_characteristics(V_REF, NTC_ADC_ATTEN, NTC_ADC_WIDTH, &characteristics);

    // initialize GPIO
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << NTC_VCC_GPIO_NUM) | (1ULL << NTC_ADC_IN_CHANNEL_GPIO_NUM),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE,
    };
    ret |= gpio_config(&io_conf);

#if defined(NTC_VCC_EN) && defined(NTC_VCC_ALWAYS_ON)
    // enable ntc
    gpio_set_direction(NTC_VCC_GPIO_NUM, GPIO_MODE_OUTPUT);
    gpio_set_level(NTC_VCC_GPIO_NUM, 1);
#endif

    // initialize rolling ave buffer to -1 to indicate it is not set
    for (int i = 0; i < ROLLING_AVE_SAMPLES_CNT; i++)
        rolling_ave[i] = -1;
#if 0
    int sample = 0;
    // initialize rolling average buffer with 1 adc sample
#if defined(NTC_VCC_EN) && !defined(NTC_VCC_ALWAYS_ON)
    // enable ntc
    gpio_set_direction(NTC_VCC_GPIO_NUM, GPIO_MODE_OUTPUT);
    gpio_set_level(NTC_VCC_GPIO_NUM, 1);
#endif
    vTaskDelay(50 / portTICK_RATE_MS);
    sample = adc1_to_voltage(NTC_ADC_IN_CHANNEL, &characteristics);
    for (int i = 0; i < ROLLING_AVE_SAMPLES_CNT; i++)
        rolling_ave[i] = sample;
#if defined(NTC_VCC_EN) && !defined(NTC_VCC_ALWAYS_ON)
    // disable ntc
    gpio_set_direction(NTC_VCC_GPIO_NUM, GPIO_MODE_INPUT);
#endif
#endif

    return ret;
}

float tempsensor_get_temperature(void) {
    int sample = 0;
    int sample_prev = 0;
    int total = 0;
    int ave = 0;
    int total_rolling_ave = 0;
    int resample_cnt = 0;

#if defined(NTC_VCC_EN) && !defined(NTC_VCC_ALWAYS_ON)
    // enable ntc
    gpio_set_direction(NTC_VCC_GPIO_NUM, GPIO_MODE_OUTPUT);
    gpio_set_level(NTC_VCC_GPIO_NUM, 1);
    vTaskDelay(50 / portTICK_RATE_MS);
#endif
    // get samples and average
//    printf("sampling start=%d\r\n", xTaskGetTickCount() * portTICK_PERIOD_MS);
    for(int i = 0; i < NTC_SAMPLES_CNT; i++) {
        resample_cnt = ADC_RESAMPLE_CNT_MAX;
        sample = adc1_to_voltage(NTC_ADC_IN_CHANNEL, &characteristics);
        if (i) {
            // resample if the sample is 1 DegC higher or lower than the previous sample
            while (((adc_to_degc(sample) - adc_to_degc(sample_prev) >= TEMP_MAX_DIFF_DEGC) \
                || (adc_to_degc(sample_prev) - adc_to_degc(sample) >= TEMP_MAX_DIFF_DEGC)) \
                && resample_cnt) {
//                printf("resample %d=%fdegC %d=%fdegC\r\n", sample, adc_to_degc(sample), sample_prev, adc_to_degc(sample_prev));
                sample = adc1_to_voltage(NTC_ADC_IN_CHANNEL, &characteristics);
                --resample_cnt;
            }
        }
//        printf("sample %d=%fdegC %d=%fdegC\r\n", sample, adc_to_degc(sample), sample_prev, adc_to_degc(sample_prev));
        sample_prev = sample;

        // get total
        total += sample;
    }
    ave = total / NTC_SAMPLES_CNT;
//    printf("ave=%d\r\n", ave);
//    printf("sampling end=%d\r\n", xTaskGetTickCount() * portTICK_PERIOD_MS);

#if defined(NTC_VCC_EN) && !defined(NTC_VCC_ALWAYS_ON)
    // disable ntc
    gpio_set_direction(NTC_VCC_GPIO_NUM, GPIO_MODE_INPUT);
#endif

    // push new average to buffer
    for (int i = ROLLING_AVE_SAMPLES_CNT - 1; i > 0; i--)
        rolling_ave[i] = rolling_ave[i - 1];
    rolling_ave[0] = ave;

    // get the total rolling averages
    for (int i = 0; i < ROLLING_AVE_SAMPLES_CNT; i++) {
//        printf("rolling_ave[%d]=%d=%f\r\n", i, rolling_ave[i], adc_to_degc(rolling_ave[i]));
        // handle unset rolling average index
        if (rolling_ave[i] == -1)
            total_rolling_ave += ave;
        else
            total_rolling_ave += rolling_ave[i];
    }

    return adc_to_degc(total_rolling_ave / ROLLING_AVE_SAMPLES_CNT);
}


static float adc_to_degc(int adc_sample) {
    float r_therm, temp1, temp2, temp3, tempf;

    r_therm = (float)NTC_R_BALANCE * ((float)1 / (float)(((float)NTC_ADC_MAX_VAL / (float)adc_sample) - (float)1));
    temp1 = (-1 * ((float)NTC_BETA / 298.15));
    temp2 = exp(temp1);
    temp3 = r_therm / ((float) NTC_R_BALANCE * temp2);
    tempf = (float)NTC_BETA / log(temp3);

    return tempf - 273.15;
}

