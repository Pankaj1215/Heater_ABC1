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

#ifndef MAIN_DISPLAY_H
#define MAIN_DISPLAY_H

#define OLED_I2C_SDA_GPIO_NUM 16
#define OLED_I2C_SCL_GPIO_NUM 17
#define OLED_I2C_FREQ_HZ 1000000
#define OLED_I2C_NUM I2C_NUM_1
#define OLED_I2C_ADDR 0x3C
#define OLED_RES_GPIO_NUM 15
#define OLED_PWR_CTL_GPIO_NUM 4

#define OLED_WIDTH SH1106_PIXEL_X
#define OLED_HEIGHT SH1106_PIXEL_Y


esp_err_t display_init(void);
esp_err_t display_clear_screen(void);
esp_err_t display_welcome_screen(int color);
esp_err_t display_standby_message(int color);
esp_err_t display_menu(char *str1, int str1_color, char *str2, int str2_color);
esp_err_t display_temperature(int val, int color);
esp_err_t display_manual_temperature_normal(int ambient_temp, int target_temp, int color);
esp_err_t display_temperature_offset_set_mode(int ambient_temp, int offset_temp, int color);
esp_err_t display_timer_value(int val, int color);
esp_err_t display_timer_mode_normal(int ambient_temp, int target_temp, int timer_min, int color);
esp_err_t display_timer_mode_changed(int timer_min, int color);
esp_err_t display_auto_mode(int ambient_temp, int target_temp, int color);
esp_err_t display_energy(int val, char *unit, int color);
esp_err_t display_year(int val, int color);
esp_err_t display_month(int val, int color);
esp_err_t display_day(int val, int color);
esp_err_t display_date(int yr, int mo, int day, int color);
esp_err_t display_hour(int val, int color);
esp_err_t display_minute(int val, int color);
esp_err_t display_time(int hr, int min, int color);
esp_err_t display_timezone(int min, int color);
esp_err_t display_ssid(char *ssid, int color);
esp_err_t display_password(char *pw, int color);
esp_err_t display_clear_last_char_ssid_pw(char ch);
esp_err_t display_password_masked(int pw_len, int color);
esp_err_t display_wps_mode_msg(int color);
esp_err_t display_brightness_val(int val, int color);
esp_err_t display_screen_timeout(int sec, int color);
esp_err_t display_child_lock_icon(int color);
esp_err_t display_timer_icon(int color);
esp_err_t display_auto_icon(int color);
esp_err_t display_wifi_icon(int color);
esp_err_t display_menu_inst(int color);
esp_err_t display_hysteresis_setting_warning(int color);
esp_err_t display_update_status(int stat, int fw_ver, int color);
esp_err_t display_update_trigger_msg(int color);
esp_err_t display_update_installing_msg(int color);
esp_err_t display_on(void);
esp_err_t display_off(void);
esp_err_t display_set_brightness(int brightness);
esp_err_t display_debug_screen_1(int temp_c, int temp_offset_c, int ambient_light, int screen_br, int pilot_light_br, int color);
esp_err_t display_debug_screen_2(bool conn, bool internet, bool kaa, bool ap_mode_en, char *ap_ssid, int color);

#endif /* MAIN_DISPLAY_H */
