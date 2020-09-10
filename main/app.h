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

#ifndef MAIN_APP_H
#define MAIN_APP_H

#include "esp_err.h"

#define MANUFACTURING_YEAR 2018
#define APP_WELCOME_SCREEN_DELAY_MS 5000
#define HEATER_OFF_LONG_PRESS_DUR_MS 2500
#define TEMP_OFFSET_LONG_PRESS_DUR_MS 2500
#define CHILD_LOCK_LONG_PRESS_DUR_MS 2500
#define DEBUG_MODE_LONG_PRESS_DUR_MS 2500
#define TIMER_MODE_RESET_TIMER_LONG_PRESS_DUR_MS 2500
#define AUTO_MODE_LONG_PRESS_DUR_MS 2500
#define MENU_MODE_LONG_PRESS_DUR_MS 2500
#define QUICK_SCROLL_LONG_PRESS_DUR_MS 2500
#define CHAR_CHANGE_TIMEOUT_MS 1000

#define APP_MODE_ON_STARTUP APP_MODE_STANDBY

#define TEMP_SENSOR_READ_INTERVAL_MS 2000
#define LIGHT_SENSOR_READ_INTERVAL_MS 1000

#define TEMPERATURE_SENSOR_OFFSET_CELSIUS_DEF -14
#define TEMPERATURE_SENSOR_OFFSET_CELSIUS_MIN -100
#define TEMPERATURE_SENSOR_OFFSET_CELSIUS_MAX 100

#define TEMPERATURE_FAHRENHEIT_VAL_MIN 32
#define TEMPERATURE_FAHRENHEIT_VAL_MAX 99
#define TEMPERATURE_FAHRENHEIT_VAL_DEF 70
#define TEMPERATURE_CELSIUS_VAL_MIN 0
#define TEMPERATURE_CELSIUS_VAL_MAX 37
#define TEMPERATURE_CELSIUS_VAL_DEF 21

#define TEMPERATURE_HYSTERESIS_CELSIUS_DEFAULT 2
#define TEMPERATURE_HYSTERESIS_CELSIUS_MAX 5
#define TEMPERATURE_HYSTERESIS_CELSIUS_MIN 2
#define TEMPERATURE_HYSTERESIS_FAHRENHEIT_DEFAULT (TEMPERATURE_HYSTERESIS_CELSIUS_DEFAULT * 9 / 5)
#define TEMPERATURE_HYSTERESIS_FAHRENHEIT_MAX (TEMPERATURE_HYSTERESIS_CELSIUS_MAX * 9 / 5)
#define TEMPERATURE_HYSTERESIS_FAHRENHEIT_MIN (TEMPERATURE_HYSTERESIS_CELSIUS_MIN * 9 / 5)

#define DISPLAY_TARGET_TEMP_DUR_MS 2000
#define DISPLAY_TIMER_DUR_MS 2000

#define TIMER_INCREMENT_MINUTES 30
#define TIMER_MIN_VALUE_MINUTES 30
#define TIMER_MAX_VALUE_MINUTES 1440

#define TIMER_EXPIRE_WAIT_FOR_INCREMENT_MS 5000

#define LAST_TIMER_SETTING_MIN_DEF 30

#define DISPLAY_COLOR 1 // 0 - black, 1 - white

//#define ENERGY_UNIT_STR "kWh"

#define NTP_SERVER "pool.ntp.org"

#define MENU_NAVIGATION_SCREEN_DISPLAY_DURATION_MS 4000

#define MENU_COMMS_BUF_MAX_LEN 32
#define MENU_COMMS_INPUT_CHARACTER_MIN_VAL 0x20 // ' '
#define MENU_COMMS_INPUT_CHARACTER_MAX_VAL 0x7e // '~'
#define MENU_COMMS_INPUT_CHARACTER_FIRST_CHAR 0x61 // 'a'

#define DISPLAY_BRIGHTNESS_MAX 100
#define DISPLAY_BRIGHTNESS_MIN 0
#define DISPLAY_BRIGHTNESS_DEF 50
#define DISPLAY_BRIGHTNESS_INCREMENT 10
#define DISPLAY_AUTO_BRIGHTNESS_UPDATE_INTERVAL_MS 1000

#define AUTO_SCREEN_OFF_DELAY_SEC_MAX 300
#define AUTO_SCREEN_OFF_DELAY_SEC_MIN 5
#define AUTO_SCREEN_OFF_DELAY_SEC_INCREMENT 5
#define AUTO_SCREEN_OFF_DELAY_SEC_DEFAULT 60

#define DIM_PILOT_LIGHT_UPDATE_INTERVAL_MS 1000
#define AUTO_DIM_PILOT_LIGHT_MAX_BRIGHTNESS 100
#define AUTO_DIM_PILOT_LIGHT_MIN_BRIGHTNESS 50

// night light
#define NIGHT_LIGHT_VAL_DEF 0x00646464
#define NIGHT_LIGHT_VAL_MIN 0x00000000
#define NIGHT_LIGHT_VAL_MAX 0x00646464
#define NIGHT_LIGHT_BRIGHTNESS_MAX 100
#define NIGHT_LIGHT_BRIGHTNESS_MIN 0
#define NIGHT_LIGHT_UPDATE_INTERVAL_MS 1000
#define NIGHT_LIGHT_BRIGHTNESS_OFF_THS 25


// AUTO Mode
#define AUTO_MODE_HEATER_ON_DUR_SEC 300
#define AUTO_MODE_HEATER_OFF_DUR_SEC 60
#define AUTO_MODE_SCHED_NUM 4
#define AUTO_MODE_INTERPOLATE 1

// Storage key
#define STORAGE_KEY_TEMP_SENSOR_OFFSET_CELSIUS "temp_off_c"
#define STORAGE_KEY_MANUAL_TEMP_CELSIUS "temp_celsius"
#define STORAGE_KEY_MANUAL_TEMP_FAHRENHEIT "temp_fahr"
#define STORAGE_KEY_SETTINGS "settings"
#define STORAGE_KEY_DISPLAY_SETTINGS "display_cfg"
#define STORAGE_KEY_SCHED_WEEKDAY "sched_wd"
#define STORAGE_KEY_SCHED_WEEKEND "sched_we"
#define STORAGE_KEY_LAST_TIMER_SETTING "last_timer"
#define STORAGE_KEY_IS_AUTO_TIME_DATE_EN "auto_dt"
#define STORAGE_KEY_TIMEZONE_OFFSET_INDEX "tz_off"
#define STORAGE_KEY_NIGHT_LIGHT_CFG "nlight_cfg"

#define DISPLAY_DATE_AFTER_CHANGE_MS 2000
#define DISPLAY_TIME_AFTER_CHANGE_MS 2000

#define DISPLAY_UPDATE_STATUS_FLASH_DURATION_MS 2000

// debug screen
#define DEBUG_SCREEN_NUM_MAX 2

#define TIMEZONE_OFFSET_INDEX_DEF 7 // -06:00


enum btn_stat {
    BUTTON_UP_STAT = 0,
    BUTTON_DOWN_STAT,
    BUTTON_POWER_BACK_STAT,
    BUTTON_TIMER_FORWARD_STAT,
};

enum menu_main {
    MENU_FIRST = 1,
//    MENU_ENERGY = 1,
    MENU_CALENDAR = 1,
    MENU_TIME_DATE,
    MENU_COMMUNICATIONS,
    MENU_SETTINGS,
    MENU_DISPLAY_SETTINGS,
    MENU_UPDATE,
    MENU_LAST = MENU_UPDATE,
};

/*
enum menu_energy {
    MENU_ENERGY_DAY = 1,
    MENU_ENERGY_WEEK,
    MENU_ENERGY_MONTH,
    MENU_ENERGY_DAY_VAL,
    MENU_ENERGY_WEEK_VAL,
    MENU_ENERGY_MONTH_VAL,
};
*/

enum menu_time_and_date {
    MENU_TIME_AND_DATE_AUTO = 1,
    MENU_TIME_AND_DATE_MANUAL,
    MENU_TIME_AND_DATE_TIMEZONE_OFFSET,
    MENU_TIME_AND_DATE_AUTO_EN,
    MENU_TIME_AND_DATE_MANUAL_DATE,
    MENU_TIME_AND_DATE_MANUAL_TIME,
    MENU_TIME_AND_DATE_TIMEZONE_OFFSET_SET,
    MENU_TIME_AND_DATE_MANUAL_DATE_YEAR,
    MENU_TIME_AND_DATE_MANUAL_DATE_MONTH,
    MENU_TIME_AND_DATE_MANUAL_DATE_DAY,
    MENU_TIME_AND_DATE_MANUAL_TIME_HOUR,
    MENU_TIME_AND_DATE_MANUAL_TIME_MINUTE,
    MENU_TIME_AND_DATE_MANUAL_DATE_YEAR_CHANGE,
    MENU_TIME_AND_DATE_MANUAL_DATE_MONTH_CHANGE,
    MENU_TIME_AND_DATE_MANUAL_DATE_DAY_CHANGE,
    MENU_TIME_AND_DATE_MANUAL_TIME_HOUR_CHANGE,
    MENU_TIME_AND_DATE_MANUAL_TIME_MINUTE_CHANGE,
};

enum menu_communications {
    MENU_COMMUNICATIONS_AP_MODE = 1,
    MENU_COMMUNICATIONS_WIFI_AP,
    MENU_COMMUNICATIONS_WPS,
    MENU_COMMUNICATIONS_AP_MODE_EN,
    MENU_COMMUNICATIONS_AP_MODE_SSID,
    MENU_COMMUNICATIONS_WIFI_AP_SSID,
    MENU_COMMUNICATIONS_WIFI_AP_PASSWORD,
    MENU_COMMUNICATIONS_WPS_EN,
    MENU_COMMUNICATIONS_AP_MODE_SSID_VAL,
    MENU_COMMUNICATIONS_WIFI_AP_SSID_CHANGE,
    MENU_COMMUNICATIONS_WIFI_AP_PASSWORD_CHANGE,
    MENU_COMMUNICATIONS_WPS_EN_INST,
};

enum menu_settings {
    MENU_SETTINGS_TEMPERATURE_UNIT = 1,
    MENU_SETTINGS_CHILD_LOCK,
    MENU_SETTINGS_PILOT_LIGHT,
    MENU_SETTINGS_NIGHT_LIGHT,
    MENU_SETTINGS_TEMPERATURE_HYSTERESIS,
    MENU_SETTINGS_TEMPERATURE_UNIT_CHANGE,
    MENU_SETTINGS_CHILD_LOCK_EN,
    MENU_SETTINGS_PILOT_LIGHT_EN,
    MENU_SETTINGS_NIGHT_LIGHT_CFG,
    MENU_SETTINGS_TEMPERATURE_HYSTERESIS_CHANGE_WARNING,
    MENU_SETTINGS_TEMPERATURE_HYSTERESIS_CHANGE,
};

enum menu_display_settings {
    MENU_DISPLAY_SETTINGS_BRIGHTNESS = 1,
    MENU_DISPLAY_SETTINGS_AUTO_SCREEN_OFF,
    MENU_DISPLAY_SETTINGS_AUTO_SCREEN_OFF_DELAY,
    MENU_DISPLAY_SETTINGS_BRIGHTNESS_MANUAL,
    MENU_DISPLAY_SETTINGS_BRIGHTNESS_AUTO,
    MENU_DISPLAY_SETTINGS_AUTO_SCREEN_OFF_EN,
    MENU_DISPLAY_SETTINGS_AUTO_SCREEN_OFF_DELAY_CHANGE,
    MENU_DISPLAY_SETTINGS_BRIGHTNESS_MANUAL_CHANGE,
    MENU_DISPLAY_SETTINGS_BRIGHTNESS_AUTO_EN,
};

enum menu_calendar {
    MENU_CALENDAR_WEEKTYPE = 1,
    MENU_CALENDAR_SCHED,
    MENU_CALENDAR_SCHED_EN,
    MENU_CALENDAR_SCHED_TIME,
    MENU_CALENDAR_SCHED_TEMP,
    MENU_CALENDAR_SCHED_EN_STAT,
    MENU_CALENDAR_SCHED_TIME_HOUR,
    MENU_CALENDAR_SCHED_TIME_MINUTE,
    MENU_CALENDAR_SCHED_TEMP_CHANGE,
    MENU_CALENDAR_SCHED_TIME_HOUR_CHANGE,
    MENU_CALENDAR_SCHED_TIME_MINUTE_CHANGE,
};

enum menu_update {
    MENU_UPDATE_STATUS = 1,
    MENU_UPDATE_TRIGGER_UPDATE,
    MENU_UPDATE_ONGOING_UPDATE,
};

typedef enum {
    APP_MODE_STANDBY = 1,
    APP_MODE_MANUAL_TEMPERATURE,
    APP_MODE_TEMPERATURE_SENSOR_OFFSET_SET,
    APP_MODE_TIMER_INCREMENT,
    APP_MODE_AUTO,
    APP_MODE_MENU,
    APP_MODE_DEBUG,
} app_mode_t;

typedef enum {
    TEMP_UNIT_CELSIUS = 0,
    TEMP_UNIT_FAHRENHEIT,
} temp_unit_t;

typedef struct {
    bool en;
    uint8_t hour;
    uint8_t minute;
    uint8_t temp_c;
    uint8_t temp_f;
} auto_mode_sched_t;

typedef struct {
    temp_unit_t temperature_unit;
    bool is_child_lock_en;
    bool is_dim_pilot_light_en;
    bool is_night_light_auto_brightness_en;
    int temperature_hysteresis_celsius;
    int temperature_hysteresis_fahrenheit;
} settings_t;

typedef struct {
    int display_brightness;
    bool is_auto_display_brightness_en;
    bool is_auto_screen_off_en;
    int auto_screen_off_delay_sec;
} display_settings_t;

typedef struct {
    app_mode_t mode;
    app_mode_t menu_mode_exit_mode;
    bool is_connected;
    bool internet_conn;
    int button_status;
    int ambient_temperature_celsius;
    int ambient_temperature_offset_celsius;
    int manual_temperature_celsius;
    int manual_temperature_fahrenheit;
    int current_timer_setting_min;
    int last_timer_setting_min;
    bool is_child_lock_active;
    bool is_auto_time_date_en;
    int timezone_offset_idx;
    int ambient_light;
    int pilot_light_brightness;
    int night_light_cfg;
    settings_t settings;
    display_settings_t display_settings;
    char ap_ssid[33];
    char ap_pw[65];
    char sta_ssid[33];
    char sta_pw[65];
} app_data_t;

/*!
 * \fn esp_err_t app_init(void)
 * \brief initialize the application thread
 * \return always success
 */
esp_err_t app_init(void);

/*!
 * \fn int app_set_mode(int mode)
 * \brief application set device mode
 * \param device mode
 * \return return success if ok
 */
int app_set_mode(int mode);

/*!
 * \fn int app_get_mode(void)
 * \brief application get device mode
 * \return return current device mode
 */
int app_get_mode(void);

/*!
 * \fn int app_get_ambient_temp(void)
 * \brief application get ambient temperature
 * \return return current temperature
 */
int app_get_ambient_temp(void);

/*!
 * \fn int app_set_target_temp(int temp_c)
 * \brief application layer set the temperature
 * \param temp_c value of temperature to be set
 * \return return success if ok
 */
int app_set_target_temp(int temp_c);

/*!
 * \fn int app_get_target_temp(void)
 * \brief get the value of the current temp
 * \return current temperature in integer
 */
int app_get_target_temp(void);

/*!
 * \fn int app_set_timer(int timer)
 * \brief set the timer 
 * \param timer value
 * \return return success if ok
 */
int app_set_timer(int timer);

/*!
 * \fn int app_get_timer(void)
 * \brief application get current timer
 * \return return current timer value
 */
int app_get_timer(void);

/*!
 * \fn int app_activate_child_lock(bool en)
 * \brief set child lock to activate
 * \param en enable or disable
 * \return return success if ok
 */
int app_activate_child_lock(bool en);

/*!
 * \fn bool app_is_child_lock_activated(void)
 * \brief get current status of child lock
 * \return return true if activated or false if not
 */
bool app_is_child_lock_activated(void);

int app_set_sched(void);
int app_get_sched(void);

/*!
 * \fn int app_enable_autoset_time_date(bool en)
 * \brief application enable auto set time and date
 * \param true if enable, false if disabled
 * \return return success if ok
 */
int app_enable_autoset_time_date(bool en);

/*!
 * \fn bool app_is_autoset_time_date_enabled(void)
 * \brief application get if autoset time date is enabled
 * \return return current state of time date
 */
bool app_is_autoset_time_date_enabled(void);

/*!
 * \fn int app_enable_ap_mode(bool en)
 * \brief application set wifi mode to ap
 * \param true if enable ap, false disabled ap
 * \return return success if ok
 */
int app_enable_ap_mode(bool en);

/*!
 * \fn bool app_is_ap_mode_enabled(void)
 * \brief application get ap mode status
 * \return return true if ap mode is enabled
 */
bool app_is_ap_mode_enabled(void);

/*!
 * \fn int int app_set_sta_mode_ssid(char *ssid, size_t len)
 * \brief application set sta mode ssid
 * \param ssid ssid of target connection
 * \param len lenght of ssid
 * \return return success if ok
 */
int app_set_sta_mode_ssid(char *ssid, size_t len);

/*!
 * \fn int app_set_sta_mode_password(char *pw, size_t len)
 * \brief application set sta mode password
 * \param pw password of target ssid
 * \param len lenght of target ap
 * \return return success if ok
 */
int app_set_sta_mode_password(char *pw, size_t len);

/*!
 * \fn int app_enable_sta_mode(bool en)
 * \brief application set sta mode enable
 * \param en true if enable, false if disabled
 * \return return success if ok
 */
int app_enable_sta_mode(bool en);

/*!
 * \fn bool app_is_sta_mode_enabled(void)
 * \brief application get if sta mode is enabled
 * \return returns true if sta mode is enabled
 */
bool app_is_sta_mode_enabled(void);

/*!
 * \fn int app_set_temp_unit(int unit)
 * \brief application set unit of temperature
 * \param unit can be celsius or fahrenheit
 * \return return success if ok
 */
int app_set_temp_unit(int unit);

/*!
 * \fn int app_get_temp_unit(void)
 * \brief application get the current unit of temperature
 * \return return celsius or fahrenheit
 */
int app_get_temp_unit(void);

/*!
 * \fn int app_enable_autodim_pilot_light(bool en)
 * \brief application enable autodim pilot light
 * \param true if enable, false if disabled
 * \return return success if ok
 */
int app_enable_autodim_pilot_light(bool en);

/*!
 * \fn bool app_is_autodim_pilot_light_enabled(void)
 * \brief application get if autodim is enabled or disabled
 * \return return true if enabled
 */
bool app_is_autodim_pilot_light_enabled(void);

/*!
 * \fn int app_enable_night_light_auto_brightness(bool en)
 * \brief application enable night light auto brightness
 * \param en enabled auto brightness
 * \return return success if ok
 */
int app_enable_night_light_auto_brightness(bool en);

/*!
 * \fn bool app_is_night_light_auto_brightness_enabled(void)
 * \brief application get if auto brightness is enabled or disabled
 * \return return true if enabled
 */
bool app_is_night_light_auto_brightness_enabled(void);

/*!
 * \fn int app_set_night_light_config(int cfg)
 * \brief application set nightlight configuration
 * \param cfg configuration 
 * \return return success if ok
 */
int app_set_night_light_config(int cfg);

/*!
 * \fn int app_get_night_light_config(void)
 * \brief application get night light config
 * \return return night light config
 */
int app_get_night_light_config(void);

/*!
 * \fn int int app_enable_child_lock(bool en)
 * \brief application set child lock enabled
 * \param en true if enable
 * \return return success if ok
 */
int app_enable_child_lock(bool en);

/*!
 * \fn bool app_is_child_lock_enabled(void)
 * \brief application get child lock if enabled or disabled
 * \return return true if child lock enabled
 */
bool app_is_child_lock_enabled(void);

/*!
 * \fn int app_enable_autodim_display(bool en)
 * \brief application set auto dim display 
 * \param en true if auto dim enabled
 * \return return success if ok
 */
int app_enable_autodim_display(bool en);

/*!
 * \fn bool app_is_autodim_display_enabled(void)
 * \brief application get if auto dim is enabled
 * \return returns true if auto dim is enabled
 */
bool app_is_autodim_display_enabled(void);

/*!
 * \fn int app_set_screen_brightness(int br)
 * \brief application set screen brightness
 * \param br brightness level
 * \return return success if ok
 */
int app_set_screen_brightness(int br);

/*!
 * \fn int app_get_screen_brightness(void)
 * \brief application get brightness level
 * \return return brightness level
 */
int app_get_screen_brightness(void);

/*!
 * \fn int app_enable_auto_screen_off(bool en)
 * \brief application set auto screen off
 * \param en true if enabled
 * \return return success if ok
 */
int app_enable_auto_screen_off(bool en);

/*!
 * \fn bool app_is_auto_screen_off_enabled(void)
 * \brief application get if auto screen off is enabled
 * \return return true if enabled
 */
bool app_is_auto_screen_off_enabled(void);

/*!
 * \fn int app_set_auto_screen_off_delay(int delay)
 * \brief application set auto screen of delay
 * \param delay duration before auto sleep
 * \return return success if ok
 */
int app_set_auto_screen_off_delay(int delay);

/*!
 * \fn int app_get_auto_screen_off_delay(void)
 * \brief application get auto sleep delay
 * \return return auto sleep delay
 */
int app_get_auto_screen_off_delay(void);

int app_start_fw_update(void);

/*!
 * \fn int app_ota_start(char* loc)
 * \brief application run ota update
 * \param loc location of the file to download
 * \return return success if ok
 */
int app_ota_start(char* loc);


#endif /* MAIN_APP_H */

