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
#include "driver/i2c.h"
#include "sdkconfig.h"

#include "display/sh1106.h"


#include "display/dejavu_sans.h"


#include "display_icon.h"
#include "display.h"


#define DISPLAY_MUTEX_LOCK(lock)    do {} while (xSemaphoreTake(lock, portMAX_DELAY) != pdPASS)
#define DISPLAY_MUTEX_UNLOCK(lock)  xSemaphoreGive(lock)


static void i2c_init(void);
void i2c_write(uint8_t reg, uint8_t *buf, size_t size);


static sh1106_t* sh1106 = NULL;
xSemaphoreHandle mutex_lock = NULL;

const char *month_str[12] = {
    "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"
};

esp_err_t display_init(void) {
    if (sh1106)
        return ESP_OK;

    if (mutex_lock == NULL)
        mutex_lock = xSemaphoreCreateMutex();

    // init comm
    i2c_init();

    // init PWR_CTL and RES
    gpio_config_t gp_cfg = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << OLED_RES_GPIO_NUM) | (1ULL << OLED_PWR_CTL_GPIO_NUM),
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&gp_cfg);

    // clear OLED_PWR_CTL to set OLED_3V3 to 3.3V
    gpio_set_level(OLED_PWR_CTL_GPIO_NUM, 0);

    // toggle OLED_RES
    gpio_set_level(OLED_RES_GPIO_NUM, 1);
    vTaskDelay(1 / portTICK_RATE_MS);
    gpio_set_level(OLED_RES_GPIO_NUM, 0);
    vTaskDelay(10 / portTICK_RATE_MS);
    gpio_set_level(OLED_RES_GPIO_NUM, 1);

    DISPLAY_MUTEX_LOCK(mutex_lock);

    // initialize OLED device
    vTaskDelay(100 / portTICK_RATE_MS);
    sh1106 = sh1106_init();
    sh1106_ret_t ret = sh1106_set_write_func(sh1106, i2c_write);
    ret |= sh1106_send_init_cmd(sh1106);

    DISPLAY_MUTEX_UNLOCK(mutex_lock);

    if (ret != SH1106_OK)
        return ESP_FAIL;

    return ESP_OK;
}

esp_err_t display_clear_screen(void) {
    sh1106_ret_t ret = SH1106_OK;

    DISPLAY_MUTEX_LOCK(mutex_lock);

    ret |= sh1106_clear_screen(sh1106);
    ret |= sh1106_update_display(sh1106);

    DISPLAY_MUTEX_UNLOCK(mutex_lock);

    if (ret != SH1106_OK)
        return ESP_FAIL;

    return ESP_OK;
}

esp_err_t display_welcome_screen(int color) {
    sh1106_ret_t ret = SH1106_OK;

    DISPLAY_MUTEX_LOCK(mutex_lock);

    ret |= sh1106_draw_image(sh1106, 0, 0, display_icon_welcome_screen->width, display_icon_welcome_screen->height, display_icon_welcome_screen->image, color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);
    ret |= sh1106_update_display(sh1106);

    DISPLAY_MUTEX_UNLOCK(mutex_lock);

    if (ret != SH1106_OK)
        return ESP_FAIL;

    return ESP_OK;
}

esp_err_t display_standby_message(int color) {
    sh1106_ret_t ret = SH1106_OK;

    DISPLAY_MUTEX_LOCK(mutex_lock);

    const char *font = DejaVu_Sans_16;
    ret |= sh1106_set_font(sh1106, font);
    int str_width1 = sh1106_get_string_width(sh1106, "Press");
    int str_width2 = sh1106_get_string_width(sh1106, "Button");
    int total_width = str_width1 + display_icon_power->width + str_width2 + 8;
    // Press
    ret |= sh1106_set_xy(sh1106, (total_width < OLED_WIDTH) ? (OLED_WIDTH - total_width) / 2 : 0, (OLED_HEIGHT - font[FONT_HEIGHT_POS]) / 2);
    ret |= sh1106_draw_string(sh1106, "Press", color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);
    // <power_icon>
    ret |= sh1106_draw_image(sh1106, (OLED_WIDTH - total_width) / 2 + str_width1 + 4, (OLED_HEIGHT - display_icon_power->height) / 2, display_icon_power->width, display_icon_power->height, display_icon_power->image, color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);
    // Button
    ret |= sh1106_set_xy(sh1106, (OLED_WIDTH - total_width) / 2 + str_width1 + display_icon_power->width + 8, (OLED_HEIGHT - font[FONT_HEIGHT_POS]) / 2);
    ret |= sh1106_draw_string(sh1106, "Button", color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);
    ret |= sh1106_update_display(sh1106);

    DISPLAY_MUTEX_UNLOCK(mutex_lock);

    if (ret != SH1106_OK)
        return ESP_FAIL;

    return ESP_OK;
}

esp_err_t display_menu(char *str1, int str1_color, char *str2, int str2_color) {
    sh1106_ret_t ret = SH1106_OK;

    DISPLAY_MUTEX_LOCK(mutex_lock);

    int str_width = 0;
    const char *font = DejaVu_Sans_24;
    ret |= sh1106_set_font(sh1106, font);
    str_width = sh1106_get_string_width(sh1106, str1);

    if (str2 == NULL) {
        ret |= sh1106_set_xy(sh1106, (str_width < OLED_WIDTH) ? (OLED_WIDTH - str_width) / 2 : 0, (OLED_HEIGHT - font[FONT_HEIGHT_POS]) / 2);
        ret |= sh1106_draw_string(sh1106, str1, str1_color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);
    } else {
        ret |= sh1106_set_xy(sh1106, (str_width < OLED_WIDTH) ? (OLED_WIDTH - str_width) / 2 : 0, OLED_HEIGHT / 2 - font[FONT_HEIGHT_POS]);
        ret |= sh1106_draw_string(sh1106, str1, str1_color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);

        str_width = sh1106_get_string_width(sh1106, str2);
        ret |= sh1106_set_xy(sh1106, (str_width < OLED_WIDTH) ? (OLED_WIDTH - str_width) / 2 : 0, OLED_HEIGHT / 2);
        ret |= sh1106_draw_string(sh1106, str2, str2_color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);
    }

    ret |= sh1106_update_display(sh1106);

    DISPLAY_MUTEX_UNLOCK(mutex_lock);

    if (ret != SH1106_OK)
        return ESP_FAIL;

    return ESP_OK;
}

esp_err_t display_temperature(int val, int color) {
    char temp_str[10];
    int str_width;
    sh1106_ret_t ret = SH1106_OK;

    DISPLAY_MUTEX_LOCK(mutex_lock);

    const char *font = DejaVu_Sans_40;
    ret |= sh1106_set_font(sh1106, font);
    sprintf(temp_str, "%d°", val);
    str_width = sh1106_get_string_width(sh1106, temp_str);
    ret |= sh1106_set_xy(sh1106, (str_width < OLED_WIDTH) ? (OLED_WIDTH - str_width) / 2 : 0, (OLED_HEIGHT - font[FONT_HEIGHT_POS]) / 2);
    ret |= sh1106_draw_string(sh1106, temp_str, color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);
    ret |= sh1106_update_display(sh1106);

    DISPLAY_MUTEX_UNLOCK(mutex_lock);

    if (ret != SH1106_OK)
        return ESP_FAIL;

    return ESP_OK;
}

esp_err_t display_manual_temperature_normal(int ambient_temp, int target_temp, int color) {
    char str[10];
    int str_width;
    const char *font;
    sh1106_ret_t ret = SH1106_OK;

    DISPLAY_MUTEX_LOCK(mutex_lock);

    // target
    font = DejaVu_Sans_40;
    ret |= sh1106_set_font(sh1106, font);
    sprintf(str, "%d°", target_temp);
    str_width = sh1106_get_string_width(sh1106, str);
    ret |= sh1106_set_xy(sh1106, (str_width < OLED_WIDTH) ? (OLED_WIDTH - str_width) / 2 : 0, (OLED_HEIGHT - font[FONT_HEIGHT_POS]) / 2);
    ret |= sh1106_draw_string(sh1106, str, color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);
    // ambient
    font = DejaVu_Sans_16;
    ret |= sh1106_set_font(sh1106, font);
    sprintf(str, "%d°", ambient_temp);
    str_width = sh1106_get_string_width(sh1106, str);
    ret |= sh1106_set_xy(sh1106, 0, OLED_HEIGHT - font[FONT_HEIGHT_POS]);
    ret |= sh1106_draw_string(sh1106, str, color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);
    ret |= sh1106_update_display(sh1106);

    DISPLAY_MUTEX_UNLOCK(mutex_lock);

    if (ret != SH1106_OK)
        return ESP_FAIL;

    return ESP_OK;
}

esp_err_t display_temperature_offset_set_mode(int ambient_temp, int offset_temp, int color) {
    char str[10];
    int str_width;
    const char *font;
    sh1106_ret_t ret = SH1106_OK;

    DISPLAY_MUTEX_LOCK(mutex_lock);

    // ambient
    font = DejaVu_Sans_40;
    ret |= sh1106_set_font(sh1106, font);
    sprintf(str, "%d°", ambient_temp);
    str_width = sh1106_get_string_width(sh1106, str);
    ret |= sh1106_set_xy(sh1106, (str_width < OLED_WIDTH) ? (OLED_WIDTH - str_width) / 2 : 0, (OLED_HEIGHT - font[FONT_HEIGHT_POS]) / 2);
    ret |= sh1106_draw_string(sh1106, str, color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);
    // "OFFSET: "
    font = DejaVu_Sans_10;
    ret |= sh1106_set_font(sh1106, font);
    ret |= sh1106_set_xy(sh1106, 0, OLED_HEIGHT - font[FONT_HEIGHT_POS]);
    ret |= sh1106_draw_string(sh1106, "OFFSET: ", color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);
    // <value>
    font = DejaVu_Sans_16;
    ret |= sh1106_set_font(sh1106, font);
    sprintf(str, "%d°", offset_temp);
    str_width = sh1106_get_string_width(sh1106, str);
    ret |= sh1106_set_xy(sh1106, sh1106->current_x, OLED_HEIGHT - font[FONT_HEIGHT_POS]);
    ret |= sh1106_draw_string(sh1106, str, color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);
    ret |= sh1106_update_display(sh1106);

    DISPLAY_MUTEX_UNLOCK(mutex_lock);

    if (ret != SH1106_OK)
        return ESP_FAIL;

    return ESP_OK;
}

esp_err_t display_timer_value(int val, int color) {
    char timer_str[10];
    int str_width;
    sh1106_ret_t ret = SH1106_OK;

    DISPLAY_MUTEX_LOCK(mutex_lock);

    const char *font = DejaVu_Sans_32;
    ret |= sh1106_set_font(sh1106, font);
    sprintf(timer_str, "%d", val);
    str_width = sh1106_get_string_width(sh1106, timer_str);
    ret |= sh1106_set_xy(sh1106, (str_width < OLED_WIDTH) ? (OLED_WIDTH - str_width) / 2 : 0, (OLED_HEIGHT - font[FONT_HEIGHT_POS]) / 2);
    ret |= sh1106_draw_string(sh1106, timer_str, color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);
    ret |= sh1106_update_display(sh1106);

    DISPLAY_MUTEX_UNLOCK(mutex_lock);

    if (ret != SH1106_OK)
        return ESP_FAIL;

    return ESP_OK;
}

esp_err_t display_timer_mode_normal(int ambient_temp, int target_temp, int timer_min, int color) {
    char str[10];
    int str_width;
    const char *font;
    sh1106_ret_t ret = SH1106_OK;
    int timer_hr = timer_min / 60;

    DISPLAY_MUTEX_LOCK(mutex_lock);

    // target temp at the center
    font = DejaVu_Sans_40;
    ret |= sh1106_set_font(sh1106, font);
    sprintf(str, "%d°", target_temp);
    str_width = sh1106_get_string_width(sh1106, str);
    ret |= sh1106_set_xy(sh1106, (str_width < OLED_WIDTH) ? (OLED_WIDTH - str_width) / 2 : 0, (OLED_HEIGHT - font[FONT_HEIGHT_POS]) / 2);
    ret |= sh1106_draw_string(sh1106, str, color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);
    // timer icon at the bottom right corner
    ret |= sh1106_draw_image(sh1106, OLED_WIDTH - display_icon_timer->width, OLED_HEIGHT - display_icon_timer->height, display_icon_timer->width, display_icon_timer->height, display_icon_timer->image, color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);

    // ambient temp at the bottom left corner
    font = DejaVu_Sans_16;
    ret |= sh1106_set_font(sh1106, font);
    sprintf(str, "%d°", ambient_temp);
    str_width = sh1106_get_string_width(sh1106, str);
    ret |= sh1106_set_xy(sh1106, 0, OLED_HEIGHT - font[FONT_HEIGHT_POS]);
    ret |= sh1106_draw_string(sh1106, str, color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);

    // timer value with unit on the left side of the timer icon
    // timer unit "hr" or "min"
    sprintf(str, "%s", timer_hr ? "hr" : "min");
    font = DejaVu_Sans_10;
    ret |= sh1106_set_font(sh1106, font);
    str_width = sh1106_get_string_width(sh1106, str);
    ret |= sh1106_set_xy(sh1106, OLED_WIDTH - display_icon_timer->width - str_width - 4, OLED_HEIGHT - font[FONT_HEIGHT_POS]);
    ret |= sh1106_draw_string(sh1106, str, color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);
    // timer value
    font = DejaVu_Sans_16;
    ret |= sh1106_set_font(sh1106, font);
    if (timer_hr)
        sprintf(str, "%0.1f", (double) timer_min / 60);
    else
        sprintf(str, "%d", timer_min);
    str_width += sh1106_get_string_width(sh1106, str);
    ret |= sh1106_set_xy(sh1106, OLED_WIDTH - display_icon_timer->width - str_width - 4, OLED_HEIGHT - font[FONT_HEIGHT_POS]);
    ret |= sh1106_draw_string(sh1106, str, color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);


    ret |= sh1106_update_display(sh1106);

    DISPLAY_MUTEX_UNLOCK(mutex_lock);

    if (ret != SH1106_OK)
        return ESP_FAIL;

    return ESP_OK;
}

esp_err_t display_timer_mode_changed(int timer_min, int color) {
    char str[10];
    int str_width;
    const char *font;
    sh1106_ret_t ret = SH1106_OK;
    int timer_hr = timer_min / 60;

    DISPLAY_MUTEX_LOCK(mutex_lock);

    // timer value at the center
    font = DejaVu_Sans_32;
    ret |= sh1106_set_font(sh1106, font);
    if (timer_hr)
        sprintf(str, "%0.1f", (double) timer_min / 60);
    else
        sprintf(str, "%d", timer_min);
    str_width = sh1106_get_string_width(sh1106, str);
    font = DejaVu_Sans_16;
    ret |= sh1106_set_font(sh1106, font);
    str_width += sh1106_get_string_width(sh1106, timer_hr ? "hr" : "min");
    font = DejaVu_Sans_32;
    ret |= sh1106_set_font(sh1106, font);
    ret |= sh1106_set_xy(sh1106, (str_width < OLED_WIDTH) ? (OLED_WIDTH - str_width) / 2 : 0, (OLED_HEIGHT - font[FONT_HEIGHT_POS]) / 2);
    ret |= sh1106_draw_string(sh1106, str, color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);
    font = DejaVu_Sans_16;
    ret |= sh1106_set_font(sh1106, font);
    ret |= sh1106_set_xy(sh1106, sh1106->current_x, sh1106->current_y + 15);
    ret |= sh1106_draw_string(sh1106, timer_hr ? "hr" : "min", color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);
    // timer icon at the bottom right corner
    ret |= sh1106_draw_image(sh1106, OLED_WIDTH - display_icon_timer->width, OLED_HEIGHT - display_icon_timer->height, display_icon_timer->width, display_icon_timer->height, display_icon_timer->image, color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);

    ret |= sh1106_update_display(sh1106);

    DISPLAY_MUTEX_UNLOCK(mutex_lock);

    if (ret != SH1106_OK)
        return ESP_FAIL;

    return ESP_OK;
}

esp_err_t display_auto_mode(int ambient_temp, int target_temp, int color) {
    char str[10];
    int str_width;
    const char *font;
    sh1106_ret_t ret = SH1106_OK;

    DISPLAY_MUTEX_LOCK(mutex_lock);

    // target
    font = DejaVu_Sans_40;
    ret |= sh1106_set_font(sh1106, font);
    sprintf(str, "%d°", target_temp);
    str_width = sh1106_get_string_width(sh1106, str);
    ret |= sh1106_set_xy(sh1106, (str_width < OLED_WIDTH) ? (OLED_WIDTH - str_width) / 2 : 0, (OLED_HEIGHT - font[FONT_HEIGHT_POS]) / 2);
    ret |= sh1106_draw_string(sh1106, str, color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);
    // ambient
    font = DejaVu_Sans_16;
    ret |= sh1106_set_font(sh1106, font);
    sprintf(str, "%d°", ambient_temp);
    str_width = sh1106_get_string_width(sh1106, str);
    ret |= sh1106_set_xy(sh1106, 0, OLED_HEIGHT - font[FONT_HEIGHT_POS]);
    ret |= sh1106_draw_string(sh1106, str, color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);
    // AUTO icon at the bottom right corner
    ret |= sh1106_draw_image(sh1106, OLED_WIDTH - display_icon_auto->width, OLED_HEIGHT - display_icon_auto->height, display_icon_auto->width, display_icon_auto->height, display_icon_auto->image, color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);
    ret |= sh1106_update_display(sh1106);

    DISPLAY_MUTEX_UNLOCK(mutex_lock);

    if (ret != SH1106_OK)
        return ESP_FAIL;

    return ESP_OK;
}

esp_err_t display_energy(int val, char *unit, int color) {
    char val_str[10];
    int str_width;
    sh1106_ret_t ret = SH1106_OK;

    const char *font_val = DejaVu_Sans_32;
    const char *font_unit = DejaVu_Sans_16;

    DISPLAY_MUTEX_LOCK(mutex_lock);

    // draw value
    ret |= sh1106_set_font(sh1106, font_val);
    sprintf(val_str, "%d", val);
    str_width = sh1106_get_string_width(sh1106, val_str);
    ret |= sh1106_set_xy(sh1106, (str_width < OLED_WIDTH) ? (OLED_WIDTH - str_width) / 2 : 0, (OLED_HEIGHT - font_val[FONT_HEIGHT_POS] - font_unit[FONT_HEIGHT_POS]) / 2);
    ret |= sh1106_draw_string(sh1106, val_str, color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);

    // draw unit
    ret |= sh1106_set_font(sh1106, font_unit);
    str_width = sh1106_get_string_width(sh1106, unit);
    ret |= sh1106_set_xy(sh1106, (str_width < OLED_WIDTH) ? (OLED_WIDTH - str_width) / 2 : 0, ((OLED_HEIGHT - font_val[FONT_HEIGHT_POS] - font_unit[FONT_HEIGHT_POS]) / 2) + font_val[FONT_HEIGHT_POS]);
    ret |= sh1106_draw_string(sh1106, unit, color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);

    // update display
    ret |= sh1106_update_display(sh1106);

    DISPLAY_MUTEX_UNLOCK(mutex_lock);

    if (ret != SH1106_OK)
        return ESP_FAIL;

    return ESP_OK;
}

esp_err_t display_year(int val, int color) {
    char val_str[10];
    int str_width;
    sh1106_ret_t ret = SH1106_OK;

    DISPLAY_MUTEX_LOCK(mutex_lock);

    const char *font = DejaVu_Sans_32;
    ret |= sh1106_set_font(sh1106, font);
    sprintf(val_str, "%d", val);
    str_width = sh1106_get_string_width(sh1106, val_str);
    ret |= sh1106_set_xy(sh1106, (str_width < OLED_WIDTH) ? (OLED_WIDTH - str_width) / 2 : 0, (OLED_HEIGHT - font[FONT_HEIGHT_POS]) / 2);
    ret |= sh1106_draw_string(sh1106, val_str, color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);
    ret |= sh1106_update_display(sh1106);

    DISPLAY_MUTEX_UNLOCK(mutex_lock);

    if (ret != SH1106_OK)
        return ESP_FAIL;

    return ESP_OK;
}

esp_err_t display_month(int val, int color) {
    int str_width;
    sh1106_ret_t ret = SH1106_OK;

    DISPLAY_MUTEX_LOCK(mutex_lock);

    const char *font = DejaVu_Sans_32;
    ret |= sh1106_set_font(sh1106, font);
    str_width = sh1106_get_string_width(sh1106, month_str[val - 1]);
    ret |= sh1106_set_xy(sh1106, (str_width < OLED_WIDTH) ? (OLED_WIDTH - str_width) / 2 : 0, (OLED_HEIGHT - font[FONT_HEIGHT_POS]) / 2);
    ret |= sh1106_draw_string(sh1106, month_str[val - 1], color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);
    ret |= sh1106_update_display(sh1106);

    DISPLAY_MUTEX_UNLOCK(mutex_lock);

    if (ret != SH1106_OK)
        return ESP_FAIL;

    return ESP_OK;
}

esp_err_t display_day(int val, int color) {
    char val_str[10];
    int str_width;
    sh1106_ret_t ret = SH1106_OK;

    DISPLAY_MUTEX_LOCK(mutex_lock);

    const char *font = DejaVu_Sans_32;
    ret |= sh1106_set_font(sh1106, font);
    sprintf(val_str, "%d", val);
    str_width = sh1106_get_string_width(sh1106, val_str);
    ret |= sh1106_set_xy(sh1106, (str_width < OLED_WIDTH) ? (OLED_WIDTH - str_width) / 2 : 0, (OLED_HEIGHT - font[FONT_HEIGHT_POS]) / 2);
    ret |= sh1106_draw_string(sh1106, val_str, color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);
    ret |= sh1106_update_display(sh1106);

    DISPLAY_MUTEX_UNLOCK(mutex_lock);

    if (ret != SH1106_OK)
        return ESP_FAIL;

    return ESP_OK;
}

esp_err_t display_date(int yr, int mo, int day, int color) {
    char val_str[10];
    int str_width;
    sh1106_ret_t ret = SH1106_OK;

    DISPLAY_MUTEX_LOCK(mutex_lock);

    const char *font = DejaVu_Sans_16;
    ret |= sh1106_set_font(sh1106, font);
    sprintf(val_str, "%d %s %02d", yr, month_str[mo - 1], day);
    str_width = sh1106_get_string_width(sh1106, val_str);
    ret |= sh1106_set_xy(sh1106, (str_width < OLED_WIDTH) ? (OLED_WIDTH - str_width) / 2 : 0, (OLED_HEIGHT - font[FONT_HEIGHT_POS]) / 2);
    ret |= sh1106_draw_string(sh1106, val_str, color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);
    ret |= sh1106_update_display(sh1106);

    DISPLAY_MUTEX_UNLOCK(mutex_lock);

    if (ret != SH1106_OK)
        return ESP_FAIL;

    return ESP_OK;
}

esp_err_t display_hour(int val, int color) {
    char val_str[10];
    int str_width;
    sh1106_ret_t ret = SH1106_OK;
    int hr;
    bool am;

    if (val == 0) {
        hr = 12;
        am = true;
    } else if (val > 12) {
	hr = val - 12;
        am = false;
    } else if (val == 12) {
        hr = 12;
        am = false;
    } else {
        hr = val;
        am = true;
    }

    DISPLAY_MUTEX_LOCK(mutex_lock);

    const char *font = DejaVu_Sans_32;
    ret |= sh1106_set_font(sh1106, font);
    sprintf(val_str, "%02d%c", hr, am ? 'a' : 'p');
    str_width = sh1106_get_string_width(sh1106, val_str);
    ret |= sh1106_set_xy(sh1106, (str_width < OLED_WIDTH) ? (OLED_WIDTH - str_width) / 2 : 0, (OLED_HEIGHT - font[FONT_HEIGHT_POS]) / 2);
    ret |= sh1106_draw_string(sh1106, val_str, color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);
    ret |= sh1106_update_display(sh1106);

    DISPLAY_MUTEX_UNLOCK(mutex_lock);

    if (ret != SH1106_OK)
        return ESP_FAIL;

    return ESP_OK;
}

esp_err_t display_minute(int val, int color) {
    char val_str[10];
    int str_width;
    sh1106_ret_t ret = SH1106_OK;

    DISPLAY_MUTEX_LOCK(mutex_lock);

    const char *font = DejaVu_Sans_32;
    ret |= sh1106_set_font(sh1106, font);
    sprintf(val_str, "%02d", val);
    str_width = sh1106_get_string_width(sh1106, val_str);
    ret |= sh1106_set_xy(sh1106, (str_width < OLED_WIDTH) ? (OLED_WIDTH - str_width) / 2 : 0, (OLED_HEIGHT - font[FONT_HEIGHT_POS]) / 2);
    ret |= sh1106_draw_string(sh1106, val_str, color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);
    ret |= sh1106_update_display(sh1106);

    DISPLAY_MUTEX_UNLOCK(mutex_lock);

    if (ret != SH1106_OK)
        return ESP_FAIL;

    return ESP_OK;
}

esp_err_t display_time(int hr, int min, int color) {
    char val_str[10];
    int str_width;
    sh1106_ret_t ret = SH1106_OK;
    int hr_dsp;
    bool am;

    if (hr == 0) {
        hr_dsp = 12;
        am = true;
    } else if (hr > 12) {
        hr_dsp = hr - 12;
        am = false;
    } else if (hr == 12) {
        hr_dsp = 12;
        am = false;
    } else {
        hr_dsp = hr;
        am = true;
    }

    DISPLAY_MUTEX_LOCK(mutex_lock);

    const char *font = DejaVu_Sans_16;
    ret |= sh1106_set_font(sh1106, font);
    sprintf(val_str, "%02d:%02d%c", hr_dsp, min, am ? 'a' : 'p');
    str_width = sh1106_get_string_width(sh1106, val_str);
    ret |= sh1106_set_xy(sh1106, (str_width < OLED_WIDTH) ? (OLED_WIDTH - str_width) / 2 : 0, (OLED_HEIGHT - font[FONT_HEIGHT_POS]) / 2);
    ret |= sh1106_draw_string(sh1106, val_str, color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);
    ret |= sh1106_update_display(sh1106);

    DISPLAY_MUTEX_UNLOCK(mutex_lock);

    if (ret != SH1106_OK)
        return ESP_FAIL;

    return ESP_OK;
}

esp_err_t display_timezone(int min, int color) {
    char val_str[10];
    int str_width;
    sh1106_ret_t ret = SH1106_OK;

    DISPLAY_MUTEX_LOCK(mutex_lock);

    const char *font = DejaVu_Sans_16;
    ret |= sh1106_set_font(sh1106, font);
    if (min == 0)
        sprintf(val_str, "±00:00");
    else
        sprintf(val_str, "%c%02d:%02d", (min > 0) ? '+' : '-', abs(min / 60), abs(min % 60));
    str_width = sh1106_get_string_width(sh1106, val_str);
    ret |= sh1106_set_xy(sh1106, (str_width < OLED_WIDTH) ? (OLED_WIDTH - str_width) / 2 : 0, (OLED_HEIGHT - font[FONT_HEIGHT_POS]) / 2);
    ret |= sh1106_draw_string(sh1106, val_str, color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);
    ret |= sh1106_update_display(sh1106);

    DISPLAY_MUTEX_UNLOCK(mutex_lock);

    if (ret != SH1106_OK)
        return ESP_FAIL;

    return ESP_OK;
}

esp_err_t display_ssid(char *ssid, int color) {
    int str_width;
    sh1106_ret_t ret = SH1106_OK;

    DISPLAY_MUTEX_LOCK(mutex_lock);

    const char *font = DejaVu_Sans_16;
    ret |= sh1106_set_font(sh1106, font);
    str_width = sh1106_get_string_width(sh1106, ssid);
    ret |= sh1106_set_xy(sh1106, (str_width < OLED_WIDTH) ? (OLED_WIDTH - str_width) / 2 : 0, (OLED_HEIGHT - font[FONT_HEIGHT_POS]) / 2);
    ret |= sh1106_draw_string(sh1106, ssid, color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);
    ret |= sh1106_update_display(sh1106);

    DISPLAY_MUTEX_UNLOCK(mutex_lock);

    if (ret != SH1106_OK)
        return ESP_FAIL;

    return ESP_OK;
}

esp_err_t display_password(char *pw, int color) {
    int str_width;
    sh1106_ret_t ret = SH1106_OK;

    DISPLAY_MUTEX_LOCK(mutex_lock);

    const char *font = DejaVu_Sans_16;
    ret |= sh1106_set_font(sh1106, font);
    str_width = sh1106_get_string_width(sh1106, pw);
    ret |= sh1106_set_xy(sh1106, (str_width < OLED_WIDTH) ? (OLED_WIDTH - str_width) / 2 : 0, (OLED_HEIGHT - font[FONT_HEIGHT_POS]) / 2);
    ret |= sh1106_draw_string(sh1106, pw, color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);
    ret |= sh1106_update_display(sh1106);

    DISPLAY_MUTEX_UNLOCK(mutex_lock);

    if (ret != SH1106_OK)
        return ESP_FAIL;

    return ESP_OK;
}

esp_err_t display_password_masked(int pw_len, int color) {
    int str_width;
    sh1106_ret_t ret = SH1106_OK;

    DISPLAY_MUTEX_LOCK(mutex_lock);

    char *pw = malloc(pw_len + 1);
    if (pw == NULL)
        return ESP_FAIL;
    memset(pw, 0, pw_len + 1);
    memset(pw, '*', pw_len);

    const char *font = DejaVu_Sans_16;
    ret |= sh1106_set_font(sh1106, font);
    str_width = sh1106_get_string_width(sh1106, pw);
    ret |= sh1106_set_xy(sh1106, (str_width < OLED_WIDTH) ? (OLED_WIDTH - str_width) / 2 : 0, (OLED_HEIGHT - font[FONT_HEIGHT_POS]) / 2);
    ret |= sh1106_draw_string(sh1106, pw, color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);
    ret |= sh1106_update_display(sh1106);

    free(pw);

    DISPLAY_MUTEX_UNLOCK(mutex_lock);

    if (ret != SH1106_OK)
        return ESP_FAIL;

    return ESP_OK;
}

esp_err_t display_clear_last_char_ssid_pw(char ch)
{
    sh1106_ret_t ret = SH1106_OK;
    int str_width;
    char ch_str[] = {ch, 0};

    DISPLAY_MUTEX_LOCK(mutex_lock);

    const char *font = DejaVu_Sans_16;
    ret |= sh1106_set_font(sh1106, font);
    str_width = sh1106_get_string_width(sh1106, ch_str);
    ret |= sh1106_set_xy(sh1106, sh1106->current_x - str_width, (OLED_HEIGHT - font[FONT_HEIGHT_POS]) / 2);
    ret |= sh1106_draw_string(sh1106, ch_str, SH1106_COLOR_BLACK);
    ret |= sh1106_update_display(sh1106);

    DISPLAY_MUTEX_UNLOCK(mutex_lock);

    if (ret != SH1106_OK)
        return ESP_FAIL;

    return ESP_OK;
}

esp_err_t display_wps_mode_msg(int color) {
    sh1106_ret_t ret = SH1106_OK;

    DISPLAY_MUTEX_LOCK(mutex_lock);

    const char *font = DejaVu_Sans_16;
    ret |= sh1106_set_font(sh1106, font);

    ret |= sh1106_set_xy(sh1106, 0, 0);
    ret |= sh1106_draw_string(sh1106, "Tap the WPS", color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);
    ret |= sh1106_set_xy(sh1106, 0, font[FONT_HEIGHT_POS]);
    ret |= sh1106_draw_string(sh1106, "button on your", color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);
    ret |= sh1106_set_xy(sh1106, 0, font[FONT_HEIGHT_POS] * 2);
    ret |= sh1106_draw_string(sh1106, "router.", color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);
    ret |= sh1106_update_display(sh1106);

    DISPLAY_MUTEX_UNLOCK(mutex_lock);

    if (ret != SH1106_OK)
        return ESP_FAIL;

    return ESP_OK;
}

esp_err_t display_brightness_val(int val, int color) {
    char val_str[10];
    int str_width;
    sh1106_ret_t ret = SH1106_OK;

    DISPLAY_MUTEX_LOCK(mutex_lock);

    const char *font = DejaVu_Sans_32;
    ret |= sh1106_set_font(sh1106, font);
    sprintf(val_str, "%d%%", val);
    str_width = sh1106_get_string_width(sh1106, val_str);
    ret |= sh1106_set_xy(sh1106, (str_width < OLED_WIDTH) ? (OLED_WIDTH - str_width) / 2 : 0, (OLED_HEIGHT - font[FONT_HEIGHT_POS]) / 2);
    ret |= sh1106_draw_string(sh1106, val_str, color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);
    ret |= sh1106_update_display(sh1106);

    DISPLAY_MUTEX_UNLOCK(mutex_lock);

    if (ret != SH1106_OK)
        return ESP_FAIL;

    return ESP_OK;
}

esp_err_t display_screen_timeout(int sec, int color) {
    char val_str[10];
    int str_width;
    sh1106_ret_t ret = SH1106_OK;

    DISPLAY_MUTEX_LOCK(mutex_lock);

    const char *font1 = DejaVu_Sans_32, *font2 = DejaVu_Sans_16;
    // timeout val
    ret |= sh1106_set_font(sh1106, font1);
    sprintf(val_str, "%d", sec);
    str_width = sh1106_get_string_width(sh1106, val_str);
    ret |= sh1106_set_xy(sh1106, (str_width < OLED_WIDTH) ? (OLED_WIDTH - str_width) / 2 : 0, (OLED_HEIGHT - font1[FONT_HEIGHT_POS] - font2[FONT_HEIGHT_POS]) / 2);
    ret |= sh1106_draw_string(sh1106, val_str, color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);
    // second / seconds
    ret |= sh1106_set_font(sh1106, font2);
    sprintf(val_str, "second%s", sec > 1 ? "s" : "");
    str_width = sh1106_get_string_width(sh1106, val_str);
    ret |= sh1106_set_xy(sh1106, (str_width < OLED_WIDTH) ? (OLED_WIDTH - str_width) / 2 : 0, (OLED_HEIGHT - font1[FONT_HEIGHT_POS] - font2[FONT_HEIGHT_POS]) / 2 + font1[FONT_HEIGHT_POS]);
    ret |= sh1106_draw_string(sh1106, val_str, color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);
    ret |= sh1106_update_display(sh1106);

    DISPLAY_MUTEX_UNLOCK(mutex_lock);

    if (ret != SH1106_OK)
        return ESP_FAIL;

    return ESP_OK;
}

esp_err_t display_child_lock_icon(int color) {
    sh1106_ret_t ret = SH1106_OK;

    DISPLAY_MUTEX_LOCK(mutex_lock);

    ret |= sh1106_draw_image(sh1106, 0, 0, display_icon_child_lock->width, display_icon_child_lock->height, display_icon_child_lock->image, color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);
    ret |= sh1106_update_display(sh1106);

    DISPLAY_MUTEX_UNLOCK(mutex_lock);

    if (ret != SH1106_OK)
        return ESP_FAIL;

    return ESP_OK;
}

esp_err_t display_timer_icon(int color) {
    sh1106_ret_t ret = SH1106_OK;

    DISPLAY_MUTEX_LOCK(mutex_lock);

    ret |= sh1106_draw_image(sh1106, 0, 0, display_icon_timer->width, display_icon_timer->height, display_icon_timer->image, color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);
    ret |= sh1106_update_display(sh1106);

    DISPLAY_MUTEX_UNLOCK(mutex_lock);

    if (ret != SH1106_OK)
        return ESP_FAIL;

    return ESP_OK;
}

esp_err_t display_auto_icon(int color) {
    sh1106_ret_t ret = SH1106_OK;

    DISPLAY_MUTEX_LOCK(mutex_lock);

    ret |= sh1106_draw_image(sh1106, OLED_WIDTH - display_icon_auto->width, OLED_HEIGHT - display_icon_auto->height, display_icon_auto->width, display_icon_auto->height, display_icon_auto->image, color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);
    ret |= sh1106_update_display(sh1106);

    DISPLAY_MUTEX_UNLOCK(mutex_lock);

    if (ret != SH1106_OK)
        return ESP_FAIL;

    return ESP_OK;
}

esp_err_t display_wifi_icon(int color) {
    sh1106_ret_t ret = SH1106_OK;

    DISPLAY_MUTEX_LOCK(mutex_lock);

    ret |= sh1106_draw_image(sh1106, OLED_WIDTH - display_icon_wifi->width, 0, display_icon_wifi->width, display_icon_wifi->height, display_icon_wifi->image, color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);
    ret |= sh1106_update_display(sh1106);

    DISPLAY_MUTEX_UNLOCK(mutex_lock);

    if (ret != SH1106_OK)
        return ESP_FAIL;

    return ESP_OK;
}

esp_err_t display_menu_inst(int color) {
    sh1106_ret_t ret = SH1106_OK;
    char str[15];

    DISPLAY_MUTEX_LOCK(mutex_lock);

    int str_width = 0;
    const char *font = DejaVu_Sans_10;
    ret |= sh1106_set_font(sh1106, font);

    sprintf(str, "%s", "Menu Navigation");
    str_width = sh1106_get_string_width(sh1106, str);
    ret |= sh1106_set_xy(sh1106, (str_width < OLED_WIDTH) ? (OLED_WIDTH - str_width) / 2 : 0, (OLED_HEIGHT - font[FONT_HEIGHT_POS]) / 2);
    ret |= sh1106_draw_string(sh1106, str, color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);
    // button labels
    font = DejaVu_Sans_16;
    ret |= sh1106_set_font(sh1106, font);
    // UP
    sprintf(str, "%s", "UP");
    ret |= sh1106_set_xy(sh1106, 0, 0);
    ret |= sh1106_draw_string(sh1106, str, color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);
    // DOWN
    sprintf(str, "%s", "DN");
    ret |= sh1106_set_xy(sh1106, 0, OLED_HEIGHT - font[FONT_HEIGHT_POS]);
    ret |= sh1106_draw_string(sh1106, str, color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);
    // BACK
    sprintf(str, "%s", "BCK");
    str_width = sh1106_get_string_width(sh1106, str);
    ret |= sh1106_set_xy(sh1106, OLED_WIDTH - str_width, 0);
    ret |= sh1106_draw_string(sh1106, str, color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);
    // FORWARD
    sprintf(str, "%s", "FWD");
    str_width = sh1106_get_string_width(sh1106, str);
    ret |= sh1106_set_xy(sh1106, OLED_WIDTH - str_width, OLED_HEIGHT - font[FONT_HEIGHT_POS]);
    ret |= sh1106_draw_string(sh1106, str, color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);
    ret |= sh1106_update_display(sh1106);

    DISPLAY_MUTEX_UNLOCK(mutex_lock);

    if (ret != SH1106_OK)
        return ESP_FAIL;

    return ESP_OK;
}

esp_err_t display_hysteresis_setting_warning(int color) {
    sh1106_ret_t ret = SH1106_OK;

    DISPLAY_MUTEX_LOCK(mutex_lock);

    const char *font1 = DejaVu_Sans_16, *font2 = DejaVu_Sans_10;
    ret |= sh1106_set_font(sh1106, font1);
    //ret |= sh1106_set_xy(sh1106, 0, 0);
    int str_width = sh1106_get_string_width(sh1106, "WARNING!!!");
    ret |= sh1106_set_xy(sh1106, (str_width < OLED_WIDTH) ? (OLED_WIDTH - str_width) / 2 : 0, 0);
    ret |= sh1106_draw_string(sh1106, "WARNING!!!", color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);
    ret |= sh1106_set_font(sh1106, font2);
    ret |= sh1106_set_xy(sh1106, 0, font1[FONT_HEIGHT_POS]);
    ret |= sh1106_draw_string(sh1106, "This is an advanced", color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);
    ret |= sh1106_set_xy(sh1106, 0, font1[FONT_HEIGHT_POS] + font2[FONT_HEIGHT_POS]);
    ret |= sh1106_draw_string(sh1106, "setting.", color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);
    ret |= sh1106_set_xy(sh1106, 0, font1[FONT_HEIGHT_POS] + font2[FONT_HEIGHT_POS] * 2);
    ret |= sh1106_draw_string(sh1106, "Proceed with caution.", color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);
    ret |= sh1106_update_display(sh1106);

    DISPLAY_MUTEX_UNLOCK(mutex_lock);

    if (ret != SH1106_OK)
        return ESP_FAIL;

    return ESP_OK;
}

esp_err_t display_update_status(int stat, int fw_ver, int color) {
    sh1106_ret_t ret = SH1106_OK;

    DISPLAY_MUTEX_LOCK(mutex_lock);

    char str1[15];
    char str2[15];
    char str3[15];
    int str_width = 0;

    if (stat) {
        sprintf(str1, "%s", "Update");
        sprintf(str2, "%d", fw_ver);
        sprintf(str3, "%s", "is available.");
    } else {
        sprintf(str1, "%s", "Firmware");
        sprintf(str2, "%s", "is");
        sprintf(str3, "%s", "up to date.");
    }

    const char *font = DejaVu_Sans_16;
    ret |= sh1106_set_font(sh1106, font);

    str_width = sh1106_get_string_width(sh1106, str1);
    ret |= sh1106_set_xy(sh1106, (str_width < OLED_WIDTH) ? (OLED_WIDTH - str_width) / 2 : 0, (OLED_HEIGHT - font[FONT_HEIGHT_POS]) / 2 - font[FONT_HEIGHT_POS]);
    ret |= sh1106_draw_string(sh1106, str1, color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);

    str_width = sh1106_get_string_width(sh1106, str2);
    ret |= sh1106_set_xy(sh1106, (str_width < OLED_WIDTH) ? (OLED_WIDTH - str_width) / 2 : 0, (OLED_HEIGHT - font[FONT_HEIGHT_POS]) / 2);
    ret |= sh1106_draw_string(sh1106, str2, color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);

    str_width = sh1106_get_string_width(sh1106, str3);
    ret |= sh1106_set_xy(sh1106, (str_width < OLED_WIDTH) ? (OLED_WIDTH - str_width) / 2 : 0, (OLED_HEIGHT - font[FONT_HEIGHT_POS]) / 2 + font[FONT_HEIGHT_POS]);
    ret |= sh1106_draw_string(sh1106, str3, color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);

    ret |= sh1106_update_display(sh1106);

    DISPLAY_MUTEX_UNLOCK(mutex_lock);

    if (ret != SH1106_OK)
        return ESP_FAIL;

    return ESP_OK;
}

esp_err_t display_update_trigger_msg(int color) {
    sh1106_ret_t ret = SH1106_OK;

    DISPLAY_MUTEX_LOCK(mutex_lock);

    char str1[15];
    char str2[15];
    char str3[15];
    int str_width = 0;

    sprintf(str1, "%s", "Do you want");
    sprintf(str2, "%s", "to install the");
    sprintf(str3, "%s", "update?");

    const char *font = DejaVu_Sans_16;
    ret |= sh1106_set_font(sh1106, font);

    str_width = sh1106_get_string_width(sh1106, str1);
    ret |= sh1106_set_xy(sh1106, (str_width < OLED_WIDTH) ? (OLED_WIDTH - str_width) / 2 : 0, (OLED_HEIGHT - font[FONT_HEIGHT_POS]) / 2 - font[FONT_HEIGHT_POS]);
    ret |= sh1106_draw_string(sh1106, str1, color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);

    str_width = sh1106_get_string_width(sh1106, str2);
    ret |= sh1106_set_xy(sh1106, (str_width < OLED_WIDTH) ? (OLED_WIDTH - str_width) / 2 : 0, (OLED_HEIGHT - font[FONT_HEIGHT_POS]) / 2);
    ret |= sh1106_draw_string(sh1106, str2, color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);

    str_width = sh1106_get_string_width(sh1106, str3);
    ret |= sh1106_set_xy(sh1106, (str_width < OLED_WIDTH) ? (OLED_WIDTH - str_width) / 2 : 0, (OLED_HEIGHT - font[FONT_HEIGHT_POS]) / 2 + font[FONT_HEIGHT_POS]);
    ret |= sh1106_draw_string(sh1106, str3, color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);

    ret |= sh1106_update_display(sh1106);

    DISPLAY_MUTEX_UNLOCK(mutex_lock);

    if (ret != SH1106_OK)
        return ESP_FAIL;

    return ESP_OK;
}

esp_err_t display_update_installing_msg(int color) {
    sh1106_ret_t ret = SH1106_OK;

    DISPLAY_MUTEX_LOCK(mutex_lock);

    char str1[15];
    char str2[15];
    int str_width = 0;

    sprintf(str1, "%s", "Installing");
    sprintf(str2, "%s", "update...");

    const char *font = DejaVu_Sans_16;
    ret |= sh1106_set_font(sh1106, font);

    str_width = sh1106_get_string_width(sh1106, str1);
    ret |= sh1106_set_xy(sh1106, (str_width < OLED_WIDTH) ? (OLED_WIDTH - str_width) / 2 : 0, OLED_HEIGHT / 2 - font[FONT_HEIGHT_POS]);
    ret |= sh1106_draw_string(sh1106, str1, color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);

    str_width = sh1106_get_string_width(sh1106, str2);
    ret |= sh1106_set_xy(sh1106, (str_width < OLED_WIDTH) ? (OLED_WIDTH - str_width) / 2 : 0, OLED_HEIGHT / 2);
    ret |= sh1106_draw_string(sh1106, str2, color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);

    ret |= sh1106_update_display(sh1106);

    DISPLAY_MUTEX_UNLOCK(mutex_lock);

    if (ret != SH1106_OK)
        return ESP_FAIL;

    return ESP_OK;
}

esp_err_t display_on(void) {
    sh1106_ret_t ret = SH1106_OK;

    DISPLAY_MUTEX_LOCK(mutex_lock);

    ret |= sh1106_display_on(sh1106);

    DISPLAY_MUTEX_UNLOCK(mutex_lock);

    if (ret != SH1106_OK)
        return ESP_FAIL;

    return ESP_OK;
}

esp_err_t display_off(void) {
    sh1106_ret_t ret = SH1106_OK;

    DISPLAY_MUTEX_LOCK(mutex_lock);

    ret |= sh1106_display_off(sh1106);

    DISPLAY_MUTEX_UNLOCK(mutex_lock);

    if (ret != SH1106_OK)
        return ESP_FAIL;

    return ESP_OK;
}

esp_err_t display_set_brightness(int brightness) {
    sh1106_ret_t ret = SH1106_OK;

    DISPLAY_MUTEX_LOCK(mutex_lock);

    ret |= sh1106_set_contrast(sh1106, brightness * 255 / 100);

    DISPLAY_MUTEX_UNLOCK(mutex_lock);

    if (ret != SH1106_OK)
        return ESP_FAIL;

    return ESP_OK;
}

esp_err_t display_debug_screen_1(int temp_c, int temp_offset_c, int ambient_light, int screen_br, int pilot_light_br, int color) {
    sh1106_ret_t ret = SH1106_OK;

    DISPLAY_MUTEX_LOCK(mutex_lock);

    char str[20];
    int y = 0;
    const char *font = DejaVu_Sans_10;
    ret |= sh1106_set_font(sh1106, font);
    // ambient temperature
    sprintf(str, "Temperature: %d°C", temp_c);
    ret |= sh1106_set_xy(sh1106, 0, y);
    ret |= sh1106_draw_string(sh1106, str, color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);
    // temperature offset
    sprintf(str, "Offset: %d°C", temp_offset_c);
    y += font[FONT_HEIGHT_POS];
    ret |= sh1106_set_xy(sh1106, 0, y);
    ret |= sh1106_draw_string(sh1106, str, color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);
    // light sensor
    sprintf(str, "Light sensor: %d%%", ambient_light);
    y += font[FONT_HEIGHT_POS];
    ret |= sh1106_set_xy(sh1106, 0, y);
    ret |= sh1106_draw_string(sh1106, str, color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);
    // display brightness percentage
    sprintf(str, "Display: %d%%", screen_br);
    y += font[FONT_HEIGHT_POS];
    ret |= sh1106_set_xy(sh1106, 0, y);
    ret |= sh1106_draw_string(sh1106, str, color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);
    // pilot light
    sprintf(str, "Pilot light: %d%%", pilot_light_br);
    y += font[FONT_HEIGHT_POS];
    ret |= sh1106_set_xy(sh1106, 0, y);
    ret |= sh1106_draw_string(sh1106, str, color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);

    ret |= sh1106_update_display(sh1106);

    DISPLAY_MUTEX_UNLOCK(mutex_lock);

    if (ret != SH1106_OK)
        return ESP_FAIL;

    return ESP_OK;
}

esp_err_t display_debug_screen_2(bool conn, bool internet, bool kaa, bool ap_mode_en, char *ap_ssid, int color) {
    sh1106_ret_t ret = SH1106_OK;

    DISPLAY_MUTEX_LOCK(mutex_lock);

    char str[20];
    int y = 0;
    const char *font = DejaVu_Sans_10;
    ret |= sh1106_set_font(sh1106, font);
    // connection status
    sprintf(str, "%s", conn ? "Connected" : "Not Connected");
    ret |= sh1106_set_xy(sh1106, 0, y);
    ret |= sh1106_draw_string(sh1106, str, color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);
    // Internet status
    sprintf(str, "Internet: %s", internet? "Yes" : "No");
    y += font[FONT_HEIGHT_POS];
    ret |= sh1106_set_xy(sh1106, 0, y);
    ret |= sh1106_draw_string(sh1106, str, color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);
    // kaa status
    sprintf(str, "Kaa: %s", kaa ? "OK" : "NOK");
    y += font[FONT_HEIGHT_POS];
    ret |= sh1106_set_xy(sh1106, 0, y);
    ret |= sh1106_draw_string(sh1106, str, color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);
    // AP mode
    sprintf(str, "AP Mode: %s", ap_mode_en ? "Enabled" : "Disabled");
    y += font[FONT_HEIGHT_POS];
    ret |= sh1106_set_xy(sh1106, 0, y);
    ret |= sh1106_draw_string(sh1106, str, color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);
    sprintf(str, "SSID: %s", ap_ssid);
    y += font[FONT_HEIGHT_POS];
    ret |= sh1106_set_xy(sh1106, 0, y);
    ret |= sh1106_draw_string(sh1106, str, color ? SH1106_COLOR_WHITE : SH1106_COLOR_BLACK);

    ret |= sh1106_update_display(sh1106);

    DISPLAY_MUTEX_UNLOCK(mutex_lock);

    if (ret != SH1106_OK)
        return ESP_FAIL;

    return ESP_OK;
}

static void i2c_init(void) {
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = OLED_I2C_SDA_GPIO_NUM;
    conf.scl_io_num = OLED_I2C_SCL_GPIO_NUM;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = OLED_I2C_FREQ_HZ;
    i2c_param_config(OLED_I2C_NUM, &conf);
    i2c_driver_install(OLED_I2C_NUM, I2C_MODE_MASTER, 0, 0, 0);
}

void i2c_write(uint8_t reg, uint8_t *buf, size_t size) {
    if (size == 0)
        return;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (OLED_I2C_ADDR << 1) | I2C_MASTER_WRITE, 1);
    i2c_master_write_byte(cmd, reg, 1);
    if (size > 1)
        i2c_master_write(cmd, buf, size, 1);
    else
        i2c_master_write_byte(cmd, *buf, 1);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(OLED_I2C_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
}

