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
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/ledc.h"
#include "sdkconfig.h"

#include "nvs_flash.h"

#include "non_volatile_lib.h"
//#include "./sdk_util/include/non_volatile_lib.h"

#include "app.h"

#include "led.h"


ledc_channel_config_t ledc_channel[LEDC_CHANNEL_CNT] = {
    {
        .channel    = LEDC_CHANNEL_0,
        .duty       = LEDC_MAX_DUTY_VAL,
        .gpio_num   = LED1_GPIO_NUM,
        .speed_mode = LEDC_SPEED_MODE,
        .timer_sel  = LEDC_TIMER_SRC
    },

    {
        .channel    = LEDC_CHANNEL_1,
        .duty       = LEDC_MAX_DUTY_VAL,
        .gpio_num   = LED2_GPIO_NUM,
        .speed_mode = LEDC_SPEED_MODE,
        .timer_sel  = LEDC_TIMER_SRC
    },

    {
        .channel    = LEDC_CHANNEL_2,
        .duty       = 0,
        .gpio_num   = NIGHT_LED_GPIO_NUM,
        .speed_mode = LEDC_SPEED_MODE,
        .timer_sel  = LEDC_TIMER_SRC
    },

    {
        .channel    = LEDC_CHANNEL_3,
#if LED_R_POL
        .duty       = 0,
#else
        .duty       = LEDC_MAX_DUTY_VAL,
#endif
        .gpio_num   = LED_R_GPIO_NUM,
        .speed_mode = LEDC_SPEED_MODE,
        .timer_sel  = LEDC_TIMER_SRC
    },

    {
        .channel    = LEDC_CHANNEL_4,
#if LED_G_POL
        .duty       = 0,
#else
        .duty       = LEDC_MAX_DUTY_VAL,
#endif
        .gpio_num   = LED_G_GPIO_NUM,
        .speed_mode = LEDC_SPEED_MODE,
        .timer_sel  = LEDC_TIMER_SRC
    },

    {
        .channel    = LEDC_CHANNEL_5,
#if LED_B_POL
        .duty       = 0,
#else
        .duty       = LEDC_MAX_DUTY_VAL,
#endif
        .gpio_num   = LED_B_GPIO_NUM,
        .speed_mode = LEDC_SPEED_MODE,
        .timer_sel  = LEDC_TIMER_SRC
    },
};


static void set_night_light_init_config(void)
{
    settings_t settings;
    settings.is_night_light_auto_brightness_en = false;
    nvs_flash_init();
    get_data_from_storage(STORAGE_KEY_SETTINGS, &settings);
    if (settings.is_night_light_auto_brightness_en == true) {
        int config;
        uint8_t r;
        uint8_t g;
        uint8_t b;
        get_integer_from_storage(STORAGE_KEY_NIGHT_LIGHT_CFG, &config);
        r = (config & 0x0000FF);
#if LED_R_POL
        r = r * LEDC_MAX_DUTY_VAL / LED_MAX_BRIGHTNESS;
#else
        r = LEDC_MAX_DUTY_VAL - (r * LEDC_MAX_DUTY_VAL / LED_MAX_BRIGHTNESS);
#endif
        ledc_channel[LED_R_CHANNEL_INDEX].duty = r;
        g = (config & 0x00FF00) >> 8;
#if LED_G_POL
        g = g * LEDC_MAX_DUTY_VAL / LED_MAX_BRIGHTNESS;
#else
        g = LEDC_MAX_DUTY_VAL - (g * LEDC_MAX_DUTY_VAL / LED_MAX_BRIGHTNESS);
#endif
        ledc_channel[LED_G_CHANNEL_INDEX].duty = g;
        b = (config & 0xFF0000) >> 16;
#if LED_B_POL
        b = b * LEDC_MAX_DUTY_VAL / LED_MAX_BRIGHTNESS;
#else
        b = LEDC_MAX_DUTY_VAL - (b * LEDC_MAX_DUTY_VAL / LED_MAX_BRIGHTNESS);
#endif
        ledc_channel[LED_B_CHANNEL_INDEX].duty = b;
    }
}


esp_err_t led_init(void) {
    // set timer configuration
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_RES,    // resolution of PWM duty
        .freq_hz = LEDC_TIMER_FREQ,           // frequency of PWM signal
        .speed_mode = LEDC_SPEED_MODE,        // timer mode
        .timer_num = LEDC_TIMER_SRC           // timer index
    };
    ledc_timer_config(&ledc_timer);

    // set initial night light setting to saved config
    set_night_light_init_config();

    // set LED controller
    for (int i = 0; i < LEDC_CHANNEL_CNT; i++)
        ledc_channel_config(&ledc_channel[i]);

    return ESP_OK;
}


esp_err_t led1_on(void) {
    return led1_set_brightness(LED_MAX_BRIGHTNESS);
}

esp_err_t led1_off(void) {
    return led1_set_brightness(0);
}

esp_err_t led1_set_brightness(uint8_t br) {
    if (br > LED_MAX_BRIGHTNESS)
        return ESP_ERR_INVALID_ARG;

    ledc_set_duty(ledc_channel[LED1_CHANNEL_INDEX].speed_mode, ledc_channel[LED1_CHANNEL_INDEX].channel, LEDC_MAX_DUTY_VAL - (br * LEDC_MAX_DUTY_VAL / LED_MAX_BRIGHTNESS));
    ledc_update_duty(ledc_channel[LED1_CHANNEL_INDEX].speed_mode, ledc_channel[LED1_CHANNEL_INDEX].channel);

    return ESP_OK;
}

esp_err_t led2_on(void) {
    return led2_set_brightness(LED_MAX_BRIGHTNESS);
}

esp_err_t led2_off(void) {
    return led2_set_brightness(0);
}

esp_err_t led2_set_brightness(uint8_t br) {
    if (br > LED_MAX_BRIGHTNESS)
        return ESP_ERR_INVALID_ARG;

    ledc_set_duty(ledc_channel[LED2_CHANNEL_INDEX].speed_mode, ledc_channel[LED2_CHANNEL_INDEX].channel, LEDC_MAX_DUTY_VAL - (br * LEDC_MAX_DUTY_VAL / LED_MAX_BRIGHTNESS));
    ledc_update_duty(ledc_channel[LED2_CHANNEL_INDEX].speed_mode, ledc_channel[LED2_CHANNEL_INDEX].channel);

    return ESP_OK;
}

esp_err_t night_led_on(void) {
    return night_led_set_brightness(LED_MAX_BRIGHTNESS);
}

esp_err_t night_led_off(void) {
    return night_led_set_brightness(0);
}

esp_err_t night_led_set_brightness(uint8_t br) {
    if (br > LED_MAX_BRIGHTNESS)
        return ESP_ERR_INVALID_ARG;

    ledc_set_duty(ledc_channel[NIGHT_LED_CHANNEL_INDEX].speed_mode, ledc_channel[NIGHT_LED_CHANNEL_INDEX].channel, br * LEDC_MAX_DUTY_VAL / LED_MAX_BRIGHTNESS);
    ledc_update_duty(ledc_channel[NIGHT_LED_CHANNEL_INDEX].speed_mode, ledc_channel[NIGHT_LED_CHANNEL_INDEX].channel);

    return ESP_OK;
}

esp_err_t led_r_on(void) {
    return led_r_set_brightness(LED_MAX_BRIGHTNESS);
}

esp_err_t led_r_off(void) {
    return led_r_set_brightness(0);
}

esp_err_t led_r_set_brightness(uint8_t br) {
    if (br > LED_MAX_BRIGHTNESS)
        return ESP_ERR_INVALID_ARG;

#if LED_R_POL
    ledc_set_duty(ledc_channel[LED_R_CHANNEL_INDEX].speed_mode, ledc_channel[LED_R_CHANNEL_INDEX].channel, br * LEDC_MAX_DUTY_VAL / LED_MAX_BRIGHTNESS);
#else
    ledc_set_duty(ledc_channel[LED_R_CHANNEL_INDEX].speed_mode, ledc_channel[LED_R_CHANNEL_INDEX].channel, LEDC_MAX_DUTY_VAL - (br * LEDC_MAX_DUTY_VAL / LED_MAX_BRIGHTNESS));
#endif
    ledc_update_duty(ledc_channel[LED_R_CHANNEL_INDEX].speed_mode, ledc_channel[LED_R_CHANNEL_INDEX].channel);

    return ESP_OK;
}

esp_err_t led_g_on(void) {
    return led_g_set_brightness(LED_MAX_BRIGHTNESS);
}

esp_err_t led_g_off(void) {
    return led_g_set_brightness(0);
}

esp_err_t led_g_set_brightness(uint8_t br) {
    if (br > LED_MAX_BRIGHTNESS)
        return ESP_ERR_INVALID_ARG;

#if LED_G_POL
    ledc_set_duty(ledc_channel[LED_G_CHANNEL_INDEX].speed_mode, ledc_channel[LED_G_CHANNEL_INDEX].channel, br * LEDC_MAX_DUTY_VAL / LED_MAX_BRIGHTNESS);
#else
    ledc_set_duty(ledc_channel[LED_G_CHANNEL_INDEX].speed_mode, ledc_channel[LED_G_CHANNEL_INDEX].channel, LEDC_MAX_DUTY_VAL - (br * LEDC_MAX_DUTY_VAL / LED_MAX_BRIGHTNESS));
#endif
    ledc_update_duty(ledc_channel[LED_G_CHANNEL_INDEX].speed_mode, ledc_channel[LED_G_CHANNEL_INDEX].channel);

    return ESP_OK;
}

esp_err_t led_b_on(void) {
    return led_b_set_brightness(LED_MAX_BRIGHTNESS);
}

esp_err_t led_b_off(void) {
    return led_b_set_brightness(0);
}

esp_err_t led_b_set_brightness(uint8_t br) {
    if (br > LED_MAX_BRIGHTNESS)
        return ESP_ERR_INVALID_ARG;

#if LED_B_POL
    ledc_set_duty(ledc_channel[LED_B_CHANNEL_INDEX].speed_mode, ledc_channel[LED_B_CHANNEL_INDEX].channel, br * LEDC_MAX_DUTY_VAL / LED_MAX_BRIGHTNESS);
#else
    ledc_set_duty(ledc_channel[LED_B_CHANNEL_INDEX].speed_mode, ledc_channel[LED_B_CHANNEL_INDEX].channel, LEDC_MAX_DUTY_VAL - (br * LEDC_MAX_DUTY_VAL / LED_MAX_BRIGHTNESS));
#endif
    ledc_update_duty(ledc_channel[LED_B_CHANNEL_INDEX].speed_mode, ledc_channel[LED_B_CHANNEL_INDEX].channel);

    return ESP_OK;
}

