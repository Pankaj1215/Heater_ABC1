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
#include "lwip/inet.h"
#include "lwip/ip4_addr.h"
#include "lwip/dns.h"
#include "ping/ping.h"
#include "esp_ping.h"
#include "sdkconfig.h"

#include "button.h"
#include "display.h"
#include "led.h"
#include "tempsensor.h"
#include "lightsensor.h"
#include "heater.h"

#include "clock/clock.h"


#include "ntp.h"
#include "communication_server.h"
#include "esp_wifi.h"

#include "non_volatile_lib.h"    // Original
// #include "./sdk_util/include/non_volatile_lib.h"   // Testing line in the code suggested by Pradeep sir

//#include "kaa_service.h"
#include "lucidtron_core.h"
#include "version.h"

#include "app.h"


// New Added _PS28Aug2020
#include "wifi_core.h"   // New Added for accessing macro #define P_TESTING which is placed in wifi_core.h


#define pilot_light_on() led1_on(); led2_on();
#define pilot_light_off() led1_off(); led2_off();
#define pilot_light_set_brightness(x) led1_set_brightness(x); led2_set_brightness(x);

// night light
#define LED_R_POS 0
#define LED_G_POS 8
#define LED_B_POS 16

#define LED_R_MASK 0x0000FF
#define LED_G_MASK 0x00FF00
#define LED_B_MASK 0xFF0000

#define GET_LED_R_VAL(BR) ((BR & LED_R_MASK) >> LED_R_POS)
#define GET_LED_G_VAL(BR) ((BR & LED_G_MASK) >> LED_G_POS)
#define GET_LED_B_VAL(BR) ((BR & LED_B_MASK) >> LED_B_POS)

#define night_light_set_br(r,g,b) {led_r_set_brightness(r);led_g_set_brightness(g);led_b_set_brightness(b);}
#define night_light_off() {led_r_off();led_g_off();led_b_off();}

#define fahr_to_celsius(f) ((f - 32) * 5 / 9)
#define celsius_to_fahr(c) (c * 9 / 5 + 32)


static void app_task(void *param);
static void standby_mode_task(app_data_t *data);
static void manual_temperature_mode_task(app_data_t *data);
static void temperature_offset_set_mode_task(app_data_t *data);
static void timer_increment_mode_task(app_data_t *data);
static void auto_mode_task(app_data_t *data);
static void debug_mode_task(app_data_t *data);
static void menu_mode_task(app_data_t *data);
static void button_up_cb(int level);
static void button_down_cb(int level);
static void button_power_back_cb(int level);
static void button_timer_forward_cb(int level);
//static app_mode_t menu_energy(app_data_t *data);
static app_mode_t menu_calendar(app_data_t *data);
static app_mode_t menu_time_and_date(app_data_t *data);
static app_mode_t menu_communications(app_data_t *data);
static app_mode_t menu_settings(app_data_t *data);
static app_mode_t menu_display_settings(app_data_t *data);
static app_mode_t menu_update(app_data_t *data);
static void temp_sensor_task(void *param);
static void light_sensor_task(void *param);
static void pilot_light_task(void *param);
static void night_light_task(void *param);
static void ping_task(void *param);
static void display_brightness_task(void *param);
static int wifi_conn_stat(int stat);


app_data_t *app_data = NULL;
static struct comm_wifi *comm_wifi_dev = NULL;
static wifi_ap_record_t ap_info;

auto_mode_sched_t sched_weekday[AUTO_MODE_SCHED_NUM];
auto_mode_sched_t sched_weekend[AUTO_MODE_SCHED_NUM];


#ifdef P_TESTING

void http_test_task(void *pvParameters);   // ADDED FOR TESTING ..PS28aUG
void aws_iot_task(void *pvParameters);   // ADDED FOR TESTING ..PS28aUG
// void http_rest_with_url();   // ADDED FOR TESTING ..PS28aUG

void initialise_wifi(void);  // Added for testing Wifi Testing _ P_16Sept2020

#endif

static int timezone_offset_list_min[] = {
    -720,        //UTC−12:00
    -660,        //UTC−11:00
    -600,        //UTC−10:00
    -570,        //UTC−09:30
    -540,        //UTC−09:00
    -480,        //UTC−08:00
    -420,        //UTC−07:00
    -360,        //UTC−06:00
    -300,        //UTC−05:00
    -240,        //UTC−04:00
    -210,        //UTC−03:30
    -180,        //UTC−03:00
    -120,        //UTC−02:00
    -60,         //UTC−01:00
    0,           //UTC±00:00
    60,          //UTC+01:00
    120,         //UTC+02:00
    180,         //UTC+03:00
    210,         //UTC+03:30
    240,         //UTC+04:00
    270,         //UTC+04:30
    300,         //UTC+05:00
    330,         //UTC+05:30
    345,         //UTC+05:45
    360,         //UTC+06:00
    390,         //UTC+06:30
    420,         //UTC+07:00
    480,         //UTC+08:00
    525,         //UTC+08:45
    540,         //UTC+09:00
    570,         //UTC+09:30
    600,         //UTC+10:00
    630,         //UTC+10:30
    660,         //UTC+11:00
    720,         //UTC+12:00
    765,         //UTC+12:45
    780,         //UTC+13:00
    840,         //UTC+14:00
};
#define TIMEZONE_OFFSET_LIST_SIZE (sizeof(timezone_offset_list_min)/sizeof(int))


static void print_fw_version(void)
{
    char fw_version[100]; 
    get_version(fw_version); 
    ESP_LOGI("firmware_version", "%s", fw_version);
}

esp_err_t app_init(void) {
    print_fw_version();

    esp_err_t ret = ESP_OK;
    time_t t_start_ms = 0;

    if (app_data != NULL)
        return ESP_OK;
    if (app_data == NULL)
        app_data = malloc(sizeof(app_data_t));
    if (app_data == NULL)
        return ESP_FAIL;

    // welcome screen
    ret |= display_clear_screen();
    ret |= display_welcome_screen(DISPLAY_COLOR);

    // system time should be after manufacturing year
    int year;
    clock_get_date(&year, NULL, NULL);
    if (year < MANUFACTURING_YEAR)
        clock_set_year(MANUFACTURING_YEAR);

    t_start_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;

    // set default values
    app_data->mode = 0;
    app_data->is_connected = false;
    app_data->internet_conn = false;
    app_data->button_status = 0;
    app_data->ambient_temperature_celsius = -1;
    app_data->ambient_temperature_offset_celsius = TEMPERATURE_SENSOR_OFFSET_CELSIUS_DEF;
    app_data->manual_temperature_celsius = TEMPERATURE_CELSIUS_VAL_DEF;
    app_data->manual_temperature_fahrenheit = TEMPERATURE_FAHRENHEIT_VAL_DEF;
    app_data->current_timer_setting_min = 0;
    app_data->last_timer_setting_min = LAST_TIMER_SETTING_MIN_DEF;
    app_data->is_child_lock_active = false;
    app_data->is_auto_time_date_en = false;
    app_data->timezone_offset_idx = TIMEZONE_OFFSET_INDEX_DEF;
    app_data->ambient_light = 0;
    app_data->pilot_light_brightness = 0;
    app_data->night_light_cfg = NIGHT_LIGHT_VAL_DEF;
    app_data->settings.temperature_unit = TEMP_UNIT_FAHRENHEIT;
    app_data->settings.is_child_lock_en = true;
    app_data->settings.is_dim_pilot_light_en = false;
    app_data->settings.is_night_light_auto_brightness_en = false;
    app_data->settings.temperature_hysteresis_celsius = TEMPERATURE_HYSTERESIS_CELSIUS_DEFAULT;
    app_data->settings.temperature_hysteresis_fahrenheit = TEMPERATURE_HYSTERESIS_FAHRENHEIT_DEFAULT;
    app_data->display_settings.display_brightness = DISPLAY_BRIGHTNESS_MAX;
    app_data->display_settings.is_auto_display_brightness_en = false;
    app_data->display_settings.is_auto_screen_off_en = true;
    app_data->display_settings.auto_screen_off_delay_sec = AUTO_SCREEN_OFF_DELAY_SEC_DEFAULT;
    memset(app_data->ap_ssid, 0, sizeof(app_data->ap_ssid));
    memset(app_data->ap_pw, 0, sizeof(app_data->ap_pw));
    memset(app_data->sta_ssid, 0, sizeof(app_data->sta_ssid));
    memset(app_data->sta_pw, 0, sizeof(app_data->sta_pw));


    // set initial values from the saved configuration from flash  // Commented for testing only
//    get_integer_from_storage(STORAGE_KEY_TEMP_SENSOR_OFFSET_CELSIUS, &(app_data->ambient_temperature_offset_celsius));
//    get_integer_from_storage(STORAGE_KEY_MANUAL_TEMP_CELSIUS, &(app_data->manual_temperature_celsius));
//    get_integer_from_storage(STORAGE_KEY_MANUAL_TEMP_FAHRENHEIT, &(app_data->manual_temperature_fahrenheit));
//    get_integer_from_storage(STORAGE_KEY_LAST_TIMER_SETTING, &(app_data->last_timer_setting_min));
//    get_integer_from_storage(STORAGE_KEY_IS_AUTO_TIME_DATE_EN, (int *) &(app_data->is_auto_time_date_en));
//    get_integer_from_storage(STORAGE_KEY_TIMEZONE_OFFSET_INDEX, &(app_data->timezone_offset_idx));
//    get_integer_from_storage(STORAGE_KEY_NIGHT_LIGHT_CFG, &(app_data->night_light_cfg));
//    get_data_from_storage(STORAGE_KEY_SETTINGS, &(app_data->settings));
//    get_data_from_storage(STORAGE_KEY_DISPLAY_SETTINGS, &(app_data->display_settings));


    clock_set_timezone_offset(timezone_offset_list_min[app_data->timezone_offset_idx]);

    // set default value of schedule
    for (int i = 0; i < AUTO_MODE_SCHED_NUM; i++) {
        sched_weekday[i].en = false;
        sched_weekday[i].hour = 0;
        sched_weekday[i].minute = 0;
        sched_weekday[i].temp_c = TEMPERATURE_CELSIUS_VAL_DEF;
        sched_weekday[i].temp_f = TEMPERATURE_FAHRENHEIT_VAL_DEF;
        sched_weekend[i].en = false;
        sched_weekend[i].hour = 0;
        sched_weekend[i].minute = 0;
        sched_weekend[i].temp_c = TEMPERATURE_CELSIUS_VAL_DEF;
        sched_weekend[i].temp_f = TEMPERATURE_FAHRENHEIT_VAL_DEF;
    }


//    // load schedule from flash  // Commneted for testing
//    get_data_from_storage(STORAGE_KEY_SCHED_WEEKDAY, sched_weekday);
//    get_data_from_storage(STORAGE_KEY_SCHED_WEEKEND, sched_weekend);



/*
    // test
    for (int i = 0; i < AUTO_MODE_SCHED_NUM; i++) {
        sched_weekday[i].en = true;
        sched_weekday[i].hour = 0;
        sched_weekday[i].minute = i;
        sched_weekday[i].temp_c = TEMPERATURE_CELSIUS_VAL_DEF + i;
        sched_weekday[i].temp_f = TEMPERATURE_FAHRENHEIT_VAL_DEF + i; //celsius_to_fahr(sched_weekday[i].temp_c);
        sched_weekend[i].en = true;
        sched_weekend[i].hour = 0;
        sched_weekend[i].minute = i;
        sched_weekend[i].temp_c = TEMPERATURE_CELSIUS_VAL_DEF + i;
        sched_weekend[i].temp_f = TEMPERATURE_FAHRENHEIT_VAL_DEF + i; //celsius_to_fahr(sched_weekend[i].temp_c);
    }
*/

    ret |= button_up_set_cb(button_up_cb);
    ret |= button_down_set_cb(button_down_cb);
    ret |= button_power_back_set_cb(button_power_back_cb);
    ret |= button_timer_forward_set_cb(button_timer_forward_cb);

    int *stat = &(app_data->button_status);
    if (button_up_get_level())
        *stat |= 1 << BUTTON_UP_STAT;

    if (button_down_get_level())
        *stat |= 1 << BUTTON_DOWN_STAT;

    if (button_power_back_get_level())
        *stat |= 1 << BUTTON_POWER_BACK_STAT;

    if (button_timer_forward_get_level())
        *stat |= 1 << BUTTON_TIMER_FORWARD_STAT;



    // Commented for Testing Pub Sub Wifi code
//    // set Wi-Fi status change callback
//    set_wifi_conn_status_change_cb(wifi_conn_stat);
//     // initialize and start communication service
//    initialize_communication_service();
//     comm_wifi_dev = get_wifi_dev();



#ifdef P_TESTING   // Added for Testing
    // onLY FOR tESTING
   // xTaskCreate(&http_test_task, "http_test_task", 8192, NULL, 5, NULL);

    initialise_wifi();

    printf("I am in main firmware \n ");
     xTaskCreate(&aws_iot_task, "aws_iot_task", 8192, NULL, 5, NULL);   // aws iot task .. initiation..
     while(1);
#endif





    // wait for at least APP_WELCOME_SCREEN_DELAY_MS
    if ((xTaskGetTickCount() * portTICK_PERIOD_MS - t_start_ms) < APP_WELCOME_SCREEN_DELAY_MS)
        vTaskDelay(APP_WELCOME_SCREEN_DELAY_MS - (xTaskGetTickCount() * portTICK_PERIOD_MS - t_start_ms) / portTICK_RATE_MS);

    // start NTP if enabled
    if (app_data->is_auto_time_date_en)
        ntp_init(NTP_SERVER);

//    // start app task  // Commented for testing
//    xTaskCreate(app_task, "app_task", 4096, (void *)app_data, 12, NULL);

/*
    // storage test
    char str_test[10] = "";
    int int_test = 0;
    get_integer_from_storage("int_test", &int_test);
    printf("int_test=%d\r\n", int_test);
    ++int_test;
    set_integer_to_storage("int_test", int_test);
    get_string_from_storage("str_test", str_test);
    printf("str_test=%s\r\n", str_test);
    sprintf(str_test, "str_test %d", int_test);
    set_string_to_storage("str_test", str_test);

    if (int_test > 1) {
        erase_integer_in_storage("int_test");
        erase_string_in_storage("str_test");
    }
*/

    return ret;
}

static void app_task(void *param) {
    app_data_t *data = (app_data_t *) param;
    app_mode_t *mode = &(data->mode);

    // start at default mode
    *mode = APP_MODE_ON_STARTUP;

//#ifdef P_TESTING
//    // onLY FOR tESTING
//   // xTaskCreate(&http_test_task, "http_test_task", 8192, NULL, 5, NULL);
//    printf("I am in main firmware \n ");
//     xTaskCreate(&aws_iot_task, "aws_iot_task", 8192, NULL, 5, NULL);   // aws iot task .. initiation..
//#endif

    // start task that reads ambient temperature
     // xTaskCreate(temp_sensor_task, "tsensor_task", 4096, (void *)app_data, 12, NULL);  // Commented only for testing _ PS27Aug20

    // start task that reads ambient light
    //  xTaskCreate(light_sensor_task, "lsensor_task", 4096, (void *)app_data, 12, NULL); // Commented only for testing _ PS27Aug20

    // start task that controls pilot light
   xTaskCreate(pilot_light_task, "plight_task", 4096, (void *)app_data, 12, NULL);  // Commented only for testing _ PS27Aug20

    // start task that controls night light
    xTaskCreate(night_light_task, "nlight_task", 4096, (void *)app_data, 12, NULL);   // Commented only for testing _ PS27Aug20

    // start task that controls display brightness
    xTaskCreate(display_brightness_task, "dbright_task", 4096, (void *)app_data, 12, NULL);   // Commented only for testing _ PS27Aug20

    while (1) {
        switch (*mode) {
        case APP_MODE_STANDBY:
        {
            printf("APP_MODE_STANDBY\r\n");
            standby_mode_task(data);
            break;
        }
        case APP_MODE_MANUAL_TEMPERATURE:
        {
            printf("APP_MODE_MANUAL_TEMPERATURE\r\n");
            manual_temperature_mode_task(data);
            break;
        }
        case APP_MODE_TEMPERATURE_SENSOR_OFFSET_SET:
        {
            printf("APP_MODE_TEMPERATURE_SENSOR_OFFSET_SET\r\n");
            temperature_offset_set_mode_task(data);
            break;
        }
        case APP_MODE_TIMER_INCREMENT:
        {
            printf("APP_MODE_TIMER_INCREMENT\r\n");
            timer_increment_mode_task((void *)data);
            break;
        }
        case APP_MODE_AUTO:
        {
            printf("APP_MODE_AUTO\r\n");
            auto_mode_task((void *)data);
            break;
        }
        case APP_MODE_MENU:
        {
            printf("APP_MODE_MENU\r\n");
            menu_mode_task((void *)data);
            break;
        }
        case APP_MODE_DEBUG:
        {
            printf("APP_MODE_DEBUG\r\n");
            debug_mode_task((void *)data);
            break;
        }
        }

        vTaskDelay(1 / portTICK_RATE_MS);
    }
} 

static void standby_mode_task(app_data_t *data) {
    int *btn = &(data->button_status);
    int prev_btn = 0;
    app_mode_t *mode = &(data->mode);
    time_t btn_up_press_ms = 0, btn_down_press_ms = 0, btn_timer_press_ms = 0;

    bool update_display = true;
    bool screen_off = false;
    time_t t_screen_on_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;

    // clear screen
    display_clear_screen();

    // wait until power button is released
    while (!((*btn >> BUTTON_POWER_BACK_STAT) & 0x01)) vTaskDelay(1 / portTICK_RATE_MS);

    // this is necessary to disregard button toggle while waiting for power button released
    prev_btn = *btn;

    // turn off heater
    heater_off();

    while(*mode == APP_MODE_STANDBY) {
        /* button
           - single press POWER button to enter Manual Temperature mode
           - long press UP and DOWN buttons to activate/deactivate child lock
           - long press DOWN and TIMER button to enter Debug mode
           - long press DOWN button to enter Menu mode
           - long press UP and TIMER buttons to enter Temperature sensor offset set mode
        */
        if (*btn == prev_btn) {
            if (data->display_settings.is_auto_screen_off_en) {
                if (((xTaskGetTickCount() * portTICK_PERIOD_MS) - t_screen_on_ms) >= (data->display_settings.auto_screen_off_delay_sec * 1000)) {
                    display_off();
                    screen_off = true;
                }
            }

            if (!screen_off) {
                int cur_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
                if ((!((*btn >> BUTTON_UP_STAT) & 0x01)) && (!((*btn >> BUTTON_DOWN_STAT) & 0x01))) { // both button up or down is pressed
                    if (((cur_ms - btn_up_press_ms) >= CHILD_LOCK_LONG_PRESS_DUR_MS)
                        && ((cur_ms - btn_down_press_ms) >= CHILD_LOCK_LONG_PRESS_DUR_MS)) {
                        if (data->settings.is_child_lock_en) {
                            data->is_child_lock_active = !data->is_child_lock_active;
                            printf("is_child_lock_active=%d\r\n", data->is_child_lock_active);

                            update_display = true;

                            btn_up_press_ms = cur_ms;
                            btn_down_press_ms = cur_ms;
                        }
                    }
                } else if ((!((*btn >> BUTTON_DOWN_STAT) & 0x01)) && (!((*btn >> BUTTON_TIMER_FORWARD_STAT) & 0x01))) { // both down and timer buttons are pressed
                    if (((cur_ms - btn_down_press_ms) >= DEBUG_MODE_LONG_PRESS_DUR_MS)
                        && ((cur_ms - btn_timer_press_ms) >= DEBUG_MODE_LONG_PRESS_DUR_MS)) {
                        if (data->settings.is_child_lock_en) {
                            *mode = APP_MODE_DEBUG;
                        }
                        btn_down_press_ms = cur_ms;
                        btn_timer_press_ms = cur_ms;
                    }
                } else if (!((*btn >> BUTTON_DOWN_STAT) & 0x01)) { // only button down is pressed
		    if ((cur_ms - btn_down_press_ms) >= MENU_MODE_LONG_PRESS_DUR_MS) {
                        if (!data->is_child_lock_active) {
                            *mode = APP_MODE_MENU;
                            data->menu_mode_exit_mode = APP_MODE_STANDBY;
                        }
                        btn_down_press_ms = cur_ms;
		    }
                } else if ((!((*btn >> BUTTON_UP_STAT) & 0x01)) && (!((*btn >> BUTTON_TIMER_FORWARD_STAT) & 0x01))) { // both up and timer buttons are pressed
                    if (((cur_ms - btn_up_press_ms) >= TEMP_OFFSET_LONG_PRESS_DUR_MS)
                        && ((cur_ms - btn_timer_press_ms) >= TEMP_OFFSET_LONG_PRESS_DUR_MS)) {
                        if (data->settings.is_child_lock_en) {
                            *mode = APP_MODE_TEMPERATURE_SENSOR_OFFSET_SET;
                        }
                        btn_up_press_ms = cur_ms;
                        btn_timer_press_ms = cur_ms;
                    }
                }
            }
	} else { // any of the buttons was pressed
	    if (data->display_settings.is_auto_screen_off_en) {
		t_screen_on_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
	    }

	    if (screen_off) {
		// enable screen only if all buttons are unpressed
		if (((*btn >> BUTTON_POWER_BACK_STAT) & 0x01) && ((*btn >> BUTTON_UP_STAT) & 0x01)
		    && ((*btn >> BUTTON_DOWN_STAT) & 0x01) && ((*btn >> BUTTON_TIMER_FORWARD_STAT) & 0x01)) {
		    screen_off = false;
		    display_on();
		}
	    } else {
		if ((*btn & (1 << BUTTON_POWER_BACK_STAT)) != (prev_btn & (1 << BUTTON_POWER_BACK_STAT))) { // power button toggles
		    if ((*btn >> BUTTON_POWER_BACK_STAT) & 0x01) { // unpressed
                        if (!data->is_child_lock_active) {
                            *mode = APP_MODE_MANUAL_TEMPERATURE;
                        }
		    }
		} else if ((*btn & (1 << BUTTON_UP_STAT)) != (prev_btn & (1 << BUTTON_UP_STAT))) { // up button toggles
                    if (!((*btn >> BUTTON_UP_STAT) & 0x01)) { // pressed
                        btn_up_press_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
                    }
                } else if ((*btn & (1 << BUTTON_DOWN_STAT)) != (prev_btn & (1 << BUTTON_DOWN_STAT))) { // down button toggles
                    if (!((*btn >> BUTTON_DOWN_STAT) & 0x01)) { // pressed
                        btn_down_press_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
                    }
                } else if ((*btn & (1 << BUTTON_TIMER_FORWARD_STAT)) != (prev_btn & (1 << BUTTON_TIMER_FORWARD_STAT))) { // timer button toggles
                    if (!((*btn >> BUTTON_TIMER_FORWARD_STAT) & 0x01)) { // pressed
                        btn_timer_press_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
                    }
                }
            }
            prev_btn = *btn;
        }

        if (update_display) {
            update_display = false;

            display_clear_screen();
            // display standby message
            display_standby_message(DISPLAY_COLOR);

            // display connection status indication if connected
            if (data->is_connected)
                display_wifi_icon(DISPLAY_COLOR);

            // display lock icon if child lock is active
            if (data->is_child_lock_active)
                display_child_lock_icon(DISPLAY_COLOR);
        }

        vTaskDelay(1 / portTICK_RATE_MS);
    }

    // exit with display on
    if (screen_off)
       display_on();
}

static void manual_temperature_mode_task(app_data_t *data) {
    int *btn = &(data->button_status);
    int prev_btn = 0;
    app_mode_t *mode = &(data->mode);
    time_t btn_power_press_ms = 0, btn_timer_press_ms = 0, btn_up_press_ms = 0, btn_down_press_ms = 0;
    int *temp = NULL, temp_max = 0, temp_min = 0;
    int *target_temp_c = &(data->manual_temperature_celsius), prev_target_temp_c = -1;
    int *ambient_temp_c = &(data->ambient_temperature_celsius), prev_ambient_temp_c = -1;
    bool is_heater_on = false;
    bool is_target_temp_changed = false;
    int *temp_hysteresis_c = &(data->settings.temperature_hysteresis_celsius);
    int *temp_hysteresis_f = &(data->settings.temperature_hysteresis_fahrenheit);
    int hysteresis_c = 0;
    temp_unit_t *temp_unit = &(data->settings.temperature_unit);

    bool update_display = true;
    bool screen_off = false;
    time_t t_screen_on_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;

    bool display_target_temp = false;
    timer_t t_target_temp_changed_ms = 0;

    // clear screen
    display_clear_screen();

    // wait until timer button is released so that it will prevent entering AUTO mode if this mode is entered from Timer mode
    while (!((*btn >> BUTTON_TIMER_FORWARD_STAT) & 0x01)) vTaskDelay(1 / portTICK_RATE_MS);

    // this is necessary to disregard button toggle while waiting for timer button released
    prev_btn = *btn;

    // set maximum and minimum temperatures and temp pointer based on unit
    if (data->settings.temperature_unit == TEMP_UNIT_CELSIUS) {
        temp_max = TEMPERATURE_CELSIUS_VAL_MAX;
        temp_min = TEMPERATURE_CELSIUS_VAL_MIN;
        temp = &(data->manual_temperature_celsius);
    } else {
        temp_max = TEMPERATURE_FAHRENHEIT_VAL_MAX;
        temp_min = TEMPERATURE_FAHRENHEIT_VAL_MIN;
        temp = &(data->manual_temperature_fahrenheit);
    }

    while(*mode == APP_MODE_MANUAL_TEMPERATURE) {
        // turn off/on the heater based on temperature
        if (*ambient_temp_c < data->manual_temperature_celsius) {
            if (!is_heater_on) {
                heater_on();
                is_heater_on = true;
                printf("MANUAL: heater on ambient=%d taget=%d\r\n", *ambient_temp_c, data->manual_temperature_celsius);
            }
        } else {
            if (*temp_unit == TEMP_UNIT_CELSIUS)
                hysteresis_c = *temp_hysteresis_c;
            else
                hysteresis_c = fahr_to_celsius(*temp_hysteresis_f);

            if (*ambient_temp_c >= (data->manual_temperature_celsius + hysteresis_c)) {
                if (is_heater_on) {
                    heater_off();
                    is_heater_on = false;
                    printf("MANUAL heater off ambient=%d taget=%d\r\n", *ambient_temp_c, data->manual_temperature_celsius);
                }
            }
        }

        if (*ambient_temp_c != prev_ambient_temp_c) {
            prev_ambient_temp_c = *ambient_temp_c;
            update_display = true;
        }

        /* button
           - single press BACK button to go back to standby mode
           - single press UP or DOWN button to increase or decrease temperature
           - long press POWER button to go to standby mode
           - long press UP AND DOWN buttons to activate/deactivate child lock
           - long press DOWN button to go to MENU mode
           - long press TIMER button to enter AUTO mode
        */
        if (*btn == prev_btn) {
            if (data->display_settings.is_auto_screen_off_en) {
                if (((xTaskGetTickCount() * portTICK_PERIOD_MS) - t_screen_on_ms) >= (data->display_settings.auto_screen_off_delay_sec * 1000)) {
                    display_off();
                    screen_off = true;
                }
            }

            if (!screen_off) {
                if (!((*btn >> BUTTON_POWER_BACK_STAT) & 0x01)) { // power button is pressed
                    int cur_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
                    if ((cur_ms - btn_power_press_ms) >= HEATER_OFF_LONG_PRESS_DUR_MS) {
                        *mode = APP_MODE_STANDBY;
                        btn_power_press_ms = cur_ms;
                    }
                } else if ((!((*btn >> BUTTON_UP_STAT) & 0x01)) && (!((*btn >> BUTTON_DOWN_STAT) & 0x01))) { // both button up AND down are pressed
                    int cur_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
                    if (((cur_ms - btn_up_press_ms) >= CHILD_LOCK_LONG_PRESS_DUR_MS)
                        && ((cur_ms - btn_down_press_ms) >= CHILD_LOCK_LONG_PRESS_DUR_MS)) {
                        if (data->settings.is_child_lock_en) {
                            data->is_child_lock_active = !data->is_child_lock_active;
                            update_display = true;
                            printf("is_child_lock_active=%d\r\n", data->is_child_lock_active);

                            btn_up_press_ms = cur_ms;
                            btn_down_press_ms = cur_ms;
                        }
                    }
                } else if (!((*btn >> BUTTON_DOWN_STAT) & 0x01)) { // only button down is pressed
                    int cur_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
                    if ((cur_ms - btn_down_press_ms) >= MENU_MODE_LONG_PRESS_DUR_MS) {
                        if (!data->is_child_lock_active) {
                            *mode = APP_MODE_MENU;
                            data->menu_mode_exit_mode = APP_MODE_MANUAL_TEMPERATURE;
                        }
                        btn_down_press_ms = cur_ms;
                    }
                } else if (!((*btn >> BUTTON_TIMER_FORWARD_STAT) & 0x01)) { // pressed
                    int cur_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
                    if ((cur_ms - btn_timer_press_ms) >= AUTO_MODE_LONG_PRESS_DUR_MS) {
                        if (!data->is_child_lock_active) {
                            *mode = APP_MODE_AUTO;
                        }
                        btn_timer_press_ms = cur_ms;
                    }
                }
            }
        } else { // any of the buttons was pressed
            if (data->display_settings.is_auto_screen_off_en) {
                t_screen_on_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
            }

            if (screen_off) {
                // enable screen only if all buttons are unpressed
                if (((*btn >> BUTTON_POWER_BACK_STAT) & 0x01) && ((*btn >> BUTTON_UP_STAT) & 0x01)
                    && ((*btn >> BUTTON_DOWN_STAT) & 0x01) && ((*btn >> BUTTON_TIMER_FORWARD_STAT) & 0x01)) {
                    screen_off = false;
                    display_on();
                }
            } else {
                if ((*btn & (1 << BUTTON_POWER_BACK_STAT)) != (prev_btn & (1 << BUTTON_POWER_BACK_STAT))) { // power button toggles
                    if ((*btn >> BUTTON_POWER_BACK_STAT) & 0x01) { // unpressed
                        if (!data->is_child_lock_active) {
                            *mode = APP_MODE_STANDBY;
                        }
                    } else {
                        btn_power_press_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
                    }
                } else if ((*btn & (1 << BUTTON_UP_STAT)) != (prev_btn & (1 << BUTTON_UP_STAT))) { // up button toggles
                    if ((*btn >> BUTTON_UP_STAT) & 0x01) { // unpressed
                        if (!data->is_child_lock_active) {
                            if ((*temp + 1) <= temp_max) {
                                is_target_temp_changed = true;
                                *temp += 1;
                            }
                        }
                    } else {
                        btn_up_press_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
                    }
                } else if ((*btn & (1 << BUTTON_DOWN_STAT)) != (prev_btn & (1 << BUTTON_DOWN_STAT))) { // down button toggles
                    if ((*btn >> BUTTON_DOWN_STAT) & 0x01) { // unpressed
                        if (!data->is_child_lock_active) {
                            if ((*temp - 1) >= temp_min) {
                                is_target_temp_changed = true;
                                *temp -= 1;
                            }
                        }
                    } else {
                        btn_down_press_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
                    }
                } else if ((*btn & (1 << BUTTON_TIMER_FORWARD_STAT)) != (prev_btn & (1 << BUTTON_TIMER_FORWARD_STAT))) { // timer button toggles
                    if ((*btn >> BUTTON_TIMER_FORWARD_STAT) & 0x01) { // unpressed
                        if (!data->is_child_lock_active) {
                            *mode = APP_MODE_TIMER_INCREMENT;
                        }
                    } else {
                        btn_timer_press_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
                    }
                }
            }
            prev_btn = *btn;
        }

        if (is_target_temp_changed) {
            is_target_temp_changed = false;

            printf("target temp=%d\r\n", *temp);

            // update the temperature of the other unit
            if (data->settings.temperature_unit == TEMP_UNIT_CELSIUS) {
                data->manual_temperature_fahrenheit = celsius_to_fahr(*temp);
            } else {
                data->manual_temperature_celsius = fahr_to_celsius(*temp);
            }

            // display target temperature
            t_target_temp_changed_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
            display_target_temp = true;
            update_display = true;

            // update target temperature in flash
            set_integer_to_storage(STORAGE_KEY_MANUAL_TEMP_CELSIUS, data->manual_temperature_celsius);
            set_integer_to_storage(STORAGE_KEY_MANUAL_TEMP_FAHRENHEIT, data->manual_temperature_fahrenheit);
        } else {
            if (display_target_temp) {
                if ((xTaskGetTickCount() * portTICK_PERIOD_MS) - t_target_temp_changed_ms >= DISPLAY_TARGET_TEMP_DUR_MS) {
                    display_target_temp = false;
                    update_display = true;
                } else {
                    update_display = false;
                }
            }
        }

        // if target temp is remotely changed, update the display
        if (*target_temp_c != prev_target_temp_c) {
            prev_target_temp_c = *target_temp_c;
            update_display = true;
        }

        if (update_display) {
            update_display = false;
            display_clear_screen();

            if (display_target_temp) {
                display_temperature(*temp, DISPLAY_COLOR);
            } else {
                display_manual_temperature_normal(data->settings.temperature_unit == TEMP_UNIT_CELSIUS ? *ambient_temp_c : celsius_to_fahr(*ambient_temp_c), *temp, DISPLAY_COLOR);
            }

            // display connected status icon if connected
            if (data->is_connected)
                display_wifi_icon(DISPLAY_COLOR);

            // display lock icon if child lock is active
            if (data->is_child_lock_active)
                display_child_lock_icon(DISPLAY_COLOR);
        }

        vTaskDelay(1 / portTICK_RATE_MS);
    }

    // exit with display on
    if (screen_off)
       display_on();
}

static void temperature_offset_set_mode_task(app_data_t *data) {
    int *btn = &(data->button_status);
    int prev_btn = 0;
    app_mode_t *mode = &(data->mode);
    time_t btn_power_press_ms = 0, btn_up_press_ms = 0, btn_down_press_ms = 0;
    int *ambient_temp_offset_c = &(data->ambient_temperature_offset_celsius);
    int *ambient_temp_c = &(data->ambient_temperature_celsius), prev_ambient_temp_c = -1;
    bool is_temp_offset_changed = false;

    bool update_display = true;
    bool screen_off = false;
    time_t t_screen_on_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;

    // clear screen
    display_clear_screen();

    // wait until up and timer buttons are released
    while ((!((*btn >> BUTTON_UP_STAT) & 0x01)) && (!((*btn >> BUTTON_TIMER_FORWARD_STAT) & 0x01)))
        vTaskDelay(1 / portTICK_RATE_MS);

    // this is necessary to disregard button toggle while waiting for up and timer buttons are released
    prev_btn = *btn;

    while(*mode == APP_MODE_TEMPERATURE_SENSOR_OFFSET_SET) {
        if (*ambient_temp_c != prev_ambient_temp_c) {
            prev_ambient_temp_c = *ambient_temp_c;
            update_display = true;
        }

        /* button
           - single press BACK button to go back to standby mode
           - single press UP or DOWN button to increase or decrease temperature offset
           - long press POWER button to go to standby mode
           - long press UP and DOWN buttons to activate/deactivate child lock
         */
        if (*btn == prev_btn) {
            if (data->display_settings.is_auto_screen_off_en) {
                if (((xTaskGetTickCount() * portTICK_PERIOD_MS) - t_screen_on_ms) >= (data->display_settings.auto_screen_off_delay_sec * 1000)) {
                    display_off();
                    screen_off = true;
                }
            }

            if (!screen_off) {
                if (!((*btn >> BUTTON_POWER_BACK_STAT) & 0x01)) { // power button is pressed
                    int cur_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
                    if ((cur_ms - btn_power_press_ms) >= HEATER_OFF_LONG_PRESS_DUR_MS) {
                        *mode = APP_MODE_STANDBY;
                        btn_power_press_ms = cur_ms;
                    }
                } else if ((!((*btn >> BUTTON_UP_STAT) & 0x01)) && (!((*btn >> BUTTON_DOWN_STAT) & 0x01))) { // both button up AND down are pressed
                    int cur_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
                    if (((cur_ms - btn_up_press_ms) >= CHILD_LOCK_LONG_PRESS_DUR_MS)
                        && ((cur_ms - btn_down_press_ms) >= CHILD_LOCK_LONG_PRESS_DUR_MS)) {
                        if (data->settings.is_child_lock_en) {
                            data->is_child_lock_active = !data->is_child_lock_active;
                            update_display = true;
                            printf("is_child_lock_active=%d\r\n", data->is_child_lock_active);

                            btn_up_press_ms = cur_ms;
                            btn_down_press_ms = cur_ms;
                        }
                    }
                }
            }
        } else { // any of the buttons was pressed
            if (data->display_settings.is_auto_screen_off_en) {
                t_screen_on_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
            }

            if (screen_off) {
                // enable screen only if all buttons are unpressed
                if (((*btn >> BUTTON_POWER_BACK_STAT) & 0x01) && ((*btn >> BUTTON_UP_STAT) & 0x01)
                    && ((*btn >> BUTTON_DOWN_STAT) & 0x01) && ((*btn >> BUTTON_TIMER_FORWARD_STAT) & 0x01)) {
                    screen_off = false;
                    display_on();
                }
            } else {
                if ((*btn & (1 << BUTTON_POWER_BACK_STAT)) != (prev_btn & (1 << BUTTON_POWER_BACK_STAT))) { // power button toggles
                    if ((*btn >> BUTTON_POWER_BACK_STAT) & 0x01) { // unpressed
                        if (!data->is_child_lock_active) {
                            *mode = APP_MODE_STANDBY;
                        }
                    } else {
                        btn_power_press_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
                    }
                } else if ((*btn & (1 << BUTTON_UP_STAT)) != (prev_btn & (1 << BUTTON_UP_STAT))) { // up button toggles
                    if ((*btn >> BUTTON_UP_STAT) & 0x01) { // unpressed
                        if (!data->is_child_lock_active) {
                            if ((*ambient_temp_offset_c + 1) <= TEMPERATURE_SENSOR_OFFSET_CELSIUS_MAX) {
                                is_temp_offset_changed = true;
                                *ambient_temp_offset_c += 1;
                            }
                        }
                    } else {
                        btn_up_press_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
                    }
                } else if ((*btn & (1 << BUTTON_DOWN_STAT)) != (prev_btn & (1 << BUTTON_DOWN_STAT))) { // down button toggles
                    if ((*btn >> BUTTON_DOWN_STAT) & 0x01) { // unpressed
                       if (!data->is_child_lock_active) {
                            if ((*ambient_temp_offset_c - 1) >= TEMPERATURE_SENSOR_OFFSET_CELSIUS_MIN) {
                                is_temp_offset_changed = true;
                                *ambient_temp_offset_c -= 1;
                            }
                        }
                    } else {
                        btn_down_press_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
                    }
                }
            }
            prev_btn = *btn;
        }

        if (is_temp_offset_changed) {
            is_temp_offset_changed = false;

            // save new offset
            set_integer_to_storage(STORAGE_KEY_TEMP_SENSOR_OFFSET_CELSIUS, *ambient_temp_offset_c);

            // update display
            update_display = true;
        }

        if (update_display) {
            update_display = false;
            display_clear_screen();

            display_temperature_offset_set_mode(*ambient_temp_c, *ambient_temp_offset_c, DISPLAY_COLOR);

            // display connected status icon if connected
            if (data->is_connected)
                display_wifi_icon(DISPLAY_COLOR);

            // display lock icon if child lock is active
            if (data->is_child_lock_active)
                display_child_lock_icon(DISPLAY_COLOR);
        }

        vTaskDelay(1 / portTICK_RATE_MS);
    }

    // exit with display on
    if (screen_off)
       display_on();
}

static void debug_mode_task(app_data_t *data) {
    int *btn = &(data->button_status);
    int prev_btn = 0;
    app_mode_t *mode = &(data->mode);
    time_t btn_power_press_ms = 0, btn_up_press_ms = 0, btn_down_press_ms = 0;

    int *temp_c = &(data->ambient_temperature_celsius), prev_temp_c = -1;
    int *temp_offset_c = &(data->ambient_temperature_offset_celsius), prev_temp_offset_c = -1;
    int *ambient_light = &(data->ambient_light), prev_ambient_light = -1;
    int *screen_br = &(data->display_settings.display_brightness), prev_screen_br = -1;
    int *pilot_light_br = &(data->pilot_light_brightness), prev_pilot_light_br = -1;

    bool *conn_stat = &(data->is_connected), prev_conn_stat = -1;
    bool *internet_conn = &(data->internet_conn), prev_internet_conn = -1;
    //TODO: bool kaa_status = is_kaa_connected();
    //TODO: bool prev_kaa_status = ~kaa_status;
    bool ap_en = -1, prev_ap_en = -1;
    char *ap_ssid = comm_wifi_dev->wifi_ap_ssid;

    bool update_display = true;
    bool screen_off = false;
    time_t t_screen_on_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;

    int screen_num = 1;

    // start ping task
    xTaskCreate(ping_task, "ping_task", 4096, (void *)data, 12, NULL);

    // clear screen
    display_clear_screen();

    // wait until up and timer buttons are released
    while ((!((*btn >> BUTTON_UP_STAT) & 0x01)) && (!((*btn >> BUTTON_TIMER_FORWARD_STAT) & 0x01)))
        vTaskDelay(1 / portTICK_RATE_MS);

    // this is necessary to disregard button toggle while waiting for up and timer buttons are released
    prev_btn = *btn;

    while(*mode == APP_MODE_DEBUG) {
        /* button
           - single press BACK button to go back to standby mode
           - long press POWER button to go to standby mode
           - long press UP and DOWN buttons to activate/deactivate child lock
         */
        if (*btn == prev_btn) {
            if (data->display_settings.is_auto_screen_off_en) {
                if (((xTaskGetTickCount() * portTICK_PERIOD_MS) - t_screen_on_ms) >= (data->display_settings.auto_screen_off_delay_sec * 1000)) {
                    display_off();
                    screen_off = true;
                }
            }

            if (!screen_off) {
                if (!((*btn >> BUTTON_POWER_BACK_STAT) & 0x01)) { // power button is pressed
                    int cur_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
                    if ((cur_ms - btn_power_press_ms) >= HEATER_OFF_LONG_PRESS_DUR_MS) {
                        *mode = APP_MODE_STANDBY;
                        btn_power_press_ms = cur_ms;
                    }
                } else if ((!((*btn >> BUTTON_UP_STAT) & 0x01)) && (!((*btn >> BUTTON_DOWN_STAT) & 0x01))) { // both button up AND down are pressed
                    int cur_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
                    if (((cur_ms - btn_up_press_ms) >= CHILD_LOCK_LONG_PRESS_DUR_MS)
                        && ((cur_ms - btn_down_press_ms) >= CHILD_LOCK_LONG_PRESS_DUR_MS)) {
                        if (data->settings.is_child_lock_en) {
                            data->is_child_lock_active = !data->is_child_lock_active;
                            update_display = true;
                            printf("is_child_lock_active=%d\r\n", data->is_child_lock_active);

                            btn_up_press_ms = cur_ms;
                            btn_down_press_ms = cur_ms;
                        }
                    }
                }
            }
        } else { // any of the buttons was pressed
            if (data->display_settings.is_auto_screen_off_en) {
                t_screen_on_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
            }

            if (screen_off) {
                // enable screen only if all buttons are unpressed
                if (((*btn >> BUTTON_POWER_BACK_STAT) & 0x01) && ((*btn >> BUTTON_UP_STAT) & 0x01)
                    && ((*btn >> BUTTON_DOWN_STAT) & 0x01) && ((*btn >> BUTTON_TIMER_FORWARD_STAT) & 0x01)) {
                    screen_off = false;
                    display_on();
                }
            } else {
                if ((*btn & (1 << BUTTON_POWER_BACK_STAT)) != (prev_btn & (1 << BUTTON_POWER_BACK_STAT))) { // power button toggles
                    if ((*btn >> BUTTON_POWER_BACK_STAT) & 0x01) { // unpressed
                        if (!data->is_child_lock_active) {
                            *mode = APP_MODE_STANDBY;
                        }
                    } else {
                        btn_power_press_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
                    }
                } else if ((*btn & (1 << BUTTON_UP_STAT)) != (prev_btn & (1 << BUTTON_UP_STAT))) { // up button toggles
                    if ((*btn >> BUTTON_UP_STAT) & 0x01) { // unpressed
                        if (screen_num < DEBUG_SCREEN_NUM_MAX) {
                            ++screen_num;
                            update_display = true;
                        }
                    } else {
                        btn_up_press_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
                    }
                } else if ((*btn & (1 << BUTTON_DOWN_STAT)) != (prev_btn & (1 << BUTTON_DOWN_STAT))) { // down button toggles
                    if ((*btn >> BUTTON_DOWN_STAT) & 0x01) { // unpressed
                        if (screen_num > 1) {
                            --screen_num;
                            update_display = true;
                        }
                    } else {
                        btn_down_press_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
                    }
                }
            }
            prev_btn = *btn;
        }

        if (screen_num == 1) {
            if (*temp_c != prev_temp_c) {
                prev_temp_c = *temp_c;
                update_display = true;
            }
            if (*temp_offset_c != prev_temp_offset_c) {
                prev_temp_offset_c = *temp_offset_c;
                update_display = true;
            }
            if (*ambient_light != prev_ambient_light) {
                prev_ambient_light = *ambient_light;
                update_display = true;
            }
            if (*screen_br != prev_screen_br) {
                prev_screen_br = *screen_br;
                update_display = true;
            }
            if (*pilot_light_br != prev_pilot_light_br) {
                prev_pilot_light_br = *pilot_light_br;
                update_display = true;
            }
        } else if (screen_num == 2) {
            if (*conn_stat != prev_conn_stat) {
                prev_conn_stat = *conn_stat;
                update_display = true;
            }
            if (*internet_conn != prev_internet_conn) {
                prev_internet_conn = *internet_conn;
                update_display = true;
            }
#if 0 //TODO:             
            kaa_status = is_kaa_connected();
            if (kaa_status != prev_kaa_status) {
                prev_kaa_status = kaa_status;
                update_display = true;
            }
#endif   
            ap_en = comm_wifi_dev->is_wifi_ap_enabled();
            if (ap_en != prev_ap_en) {
                prev_ap_en = ap_en;
                update_display = true;
            }
        }

        if (update_display) {
            update_display = false;
            display_clear_screen();

            if (screen_num == 1)
                display_debug_screen_1(*temp_c, *temp_offset_c, *ambient_light, *screen_br, *pilot_light_br, DISPLAY_COLOR);
            //TODO: else if (screen_num == 2)
                //display_debug_screen_2(*conn_stat, *internet_conn, kaa_status, ap_en, ap_ssid, DISPLAY_COLOR);

            // display lock icon if child lock is active
            if (data->is_child_lock_active)
                display_child_lock_icon(DISPLAY_COLOR);
        }

        vTaskDelay(1 / portTICK_RATE_MS);
    }

    // exit with display on
    if (screen_off)
       display_on();
}

static void timer_increment_mode_task(app_data_t *data) {
    int *btn = &(data->button_status);
    int prev_btn = *btn;
    app_mode_t *mode = &(data->mode);
    time_t btn_power_press_ms = 0, btn_up_press_ms = 0, btn_down_press_ms = 0, btn_timer_press_ms = 0;
    int *timer_min = &(data->current_timer_setting_min), prev_timer_min = -1;
    time_t t0_ms = 0, t1_ms = 0;
    int *target_temp_c = &(data->manual_temperature_celsius), *target_temp_f = &(data->manual_temperature_fahrenheit);
    int prev_target_temp_c = -1;
    int *ambient_temp_c = &(data->ambient_temperature_celsius), prev_ambient_temp_c = -1;
    bool is_heater_on = false;
    int *temp_hysteresis_c = &(data->settings.temperature_hysteresis_celsius);
    int *temp_hysteresis_f = &(data->settings.temperature_hysteresis_fahrenheit);
    int hysteresis_c = 0;
    temp_unit_t *temp_unit = &(data->settings.temperature_unit);
    int *temp = NULL, temp_max = 0, temp_min = 0;
    bool is_target_temp_changed = false;
    timer_t t_target_temp_changed_ms = 0;
    bool display_target_temp = false;

    bool update_display = true;
    bool screen_off = false;
    time_t t_screen_on_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
    bool is_timer_change_en = true; // whether timer can be increased or decreased
    bool is_timer_changed = true; // whether timer is increased or decreased
    time_t t_timer_changed_ms = 0;
    bool display_timer = false;

    time_t t_timer_expire_ms = 0;
    bool is_timer_expired = false;

    // set inital timer value
    *timer_min = data->last_timer_setting_min;
    t0_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;

    // set maximum and minimum temperatures and temp pointer based on unit
    if (data->settings.temperature_unit == TEMP_UNIT_CELSIUS) {
        temp_max = TEMPERATURE_CELSIUS_VAL_MAX;
        temp_min = TEMPERATURE_CELSIUS_VAL_MIN;
        temp = &(data->manual_temperature_celsius);
    } else {
        temp_max = TEMPERATURE_FAHRENHEIT_VAL_MAX;
        temp_min = TEMPERATURE_FAHRENHEIT_VAL_MIN;
        temp = &(data->manual_temperature_fahrenheit);
    }

    while(*mode == APP_MODE_TIMER_INCREMENT) {
        if (*timer_min == 0) {
            if (is_timer_expired) {
                // go to standby only if there is no timer increment within 5 seconds
                if (((xTaskGetTickCount() * portTICK_PERIOD_MS) - t_timer_expire_ms) >= (TIMER_EXPIRE_WAIT_FOR_INCREMENT_MS)) {
                    *mode = APP_MODE_STANDBY;
                }
            } else {
                is_timer_expired = true;

                // set time when the timer expired
                t_timer_expire_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
            }

            // turn off heater
            heater_off();
            is_heater_on = false;
        } else {
            is_timer_expired = false;

            // update timer based on lapsed time
            t1_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
            if ((t1_ms - t0_ms) >= 60000) { // 1 minute has passed
                *timer_min -= 1;
                t0_ms = t1_ms;
            }

            // turn off/on the heater based on temperature
            if (*ambient_temp_c < *target_temp_c) {
                if (!is_heater_on) {
                    heater_on();
                    is_heater_on = true;
                    printf("TIMER: heater on ambient=%d taget=%d\r\n", *ambient_temp_c, *target_temp_c);
                }
            } else {
                if (*temp_unit == TEMP_UNIT_CELSIUS)
                    hysteresis_c = *temp_hysteresis_c;
                else
                    hysteresis_c = fahr_to_celsius(*temp_hysteresis_f);

                if (*ambient_temp_c >= (*target_temp_c + hysteresis_c)) {
                    if (is_heater_on) {
                        heater_off();
                        is_heater_on = false;
                        printf("TIMER: heater off ambient=%d taget=%d\r\n", *ambient_temp_c, *target_temp_c);
                    }
                }
            }
        }

        /* button
           - single press TIMER button to enable increasing or increasing timer value
           - single press UP/DOWN button to increase/decrease target temperature if changing timer value is not enabled
           - single press UP/DOWN button to increase/decrease timer setting if changing timer setting is enabled
           - long press POWER button to go to standby mode
           - long press DOWN button to go to MENU mode
           - long press TIMER button to reset timer and go to manual temperature mode
        */
        if (*btn == prev_btn) {
            if (data->display_settings.is_auto_screen_off_en) {
                if (((xTaskGetTickCount() * portTICK_PERIOD_MS) - t_screen_on_ms) >= (data->display_settings.auto_screen_off_delay_sec * 1000)) {
                    display_off();
                    screen_off = true;
                }
            }

            if (!screen_off) {
                if (!((*btn >> BUTTON_POWER_BACK_STAT) & 0x01)) { // power button is pressed
                    int cur_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
                    if ((cur_ms - btn_power_press_ms) >= HEATER_OFF_LONG_PRESS_DUR_MS) {
                        *mode = APP_MODE_STANDBY;
                        btn_power_press_ms = cur_ms;
                    }
                } else if ((!((*btn >> BUTTON_UP_STAT) & 0x01)) && (!((*btn >> BUTTON_DOWN_STAT) & 0x01))) { // both button up AND down are pressed
                    int cur_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
                    if (((cur_ms - btn_up_press_ms) >= CHILD_LOCK_LONG_PRESS_DUR_MS)
                        && ((cur_ms - btn_down_press_ms) >= CHILD_LOCK_LONG_PRESS_DUR_MS)) {
                        if (data->settings.is_child_lock_en) {
                            data->is_child_lock_active = !data->is_child_lock_active;
                            update_display = true;
                            printf("is_child_lock_active=%d\r\n", data->is_child_lock_active);

                            btn_up_press_ms = cur_ms;
                            btn_down_press_ms = cur_ms;
                        }
                    }
                } else if (!((*btn >> BUTTON_DOWN_STAT) & 0x01)) { // only button down is pressed
                    int cur_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
                    if ((cur_ms - btn_down_press_ms) >= MENU_MODE_LONG_PRESS_DUR_MS) {
                        if (!data->is_child_lock_active) {
                            *mode = APP_MODE_MENU;
                            data->menu_mode_exit_mode = APP_MODE_TIMER_INCREMENT;
                        }
                        btn_down_press_ms = cur_ms;
                    }
                } else if (!((*btn >> BUTTON_TIMER_FORWARD_STAT) & 0x01)) { // pressed
                    int cur_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
                    if ((cur_ms - btn_timer_press_ms) >= TIMER_MODE_RESET_TIMER_LONG_PRESS_DUR_MS) {
                        if (!data->is_child_lock_active) {
                            *timer_min = 0;
                            *mode = APP_MODE_MANUAL_TEMPERATURE;
                        }
                        btn_timer_press_ms = cur_ms;
                    }
                }
            }
        } else { // any of the buttons was pressed
            if (data->display_settings.is_auto_screen_off_en) {
                t_screen_on_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
            }

            if (screen_off) {
                // enable screen only if all buttons are unpressed
                if (((*btn >> BUTTON_POWER_BACK_STAT) & 0x01) && ((*btn >> BUTTON_UP_STAT) & 0x01)
                    && ((*btn >> BUTTON_DOWN_STAT) & 0x01) && ((*btn >> BUTTON_TIMER_FORWARD_STAT) & 0x01)) {
                    screen_off = false;
                    display_on();
                }
            } else {
                if ((*btn & (1 << BUTTON_POWER_BACK_STAT)) != (prev_btn & (1 << BUTTON_POWER_BACK_STAT))) { // power button toggles
                    if (!((*btn >> BUTTON_POWER_BACK_STAT) & 0x01)) { // pressed
                        btn_power_press_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
                    }
                } else if ((*btn & (1 << BUTTON_UP_STAT)) != (prev_btn & (1 << BUTTON_UP_STAT))) { // up button toggles
                    if ((*btn >> BUTTON_UP_STAT) & 0x01) { // unpressed
                        if (!data->is_child_lock_active) {
                            if (is_timer_change_en) {
                                is_timer_changed = true;
                                int temp_timer_min = (*timer_min / TIMER_INCREMENT_MINUTES) * TIMER_INCREMENT_MINUTES + TIMER_INCREMENT_MINUTES;
                                if (temp_timer_min < TIMER_MAX_VALUE_MINUTES)
                                    *timer_min = temp_timer_min;
                                else
                                    *timer_min = TIMER_MAX_VALUE_MINUTES;
                            } else {
                                if ((*temp + 1) <= temp_max) {
                                    is_target_temp_changed = true;
                                    *temp += 1;
                                }
                            }
                        }
                    } else { // pressed
                        btn_up_press_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
                    }
                } else if ((*btn & (1 << BUTTON_DOWN_STAT)) != (prev_btn & (1 << BUTTON_DOWN_STAT))) { // down button toggles
                    if ((*btn >> BUTTON_DOWN_STAT) & 0x01) { // unpressed
                        if (!data->is_child_lock_active) {
                            if (is_timer_change_en) {
                                if (*timer_min > TIMER_MIN_VALUE_MINUTES) {
                                    is_timer_changed = true;
                                    int temp_timer_min = (*timer_min % TIMER_INCREMENT_MINUTES  == 0) ? *timer_min - TIMER_INCREMENT_MINUTES
                                        : (*timer_min / TIMER_INCREMENT_MINUTES) * TIMER_INCREMENT_MINUTES;
                                    if (temp_timer_min > TIMER_MIN_VALUE_MINUTES)
                                        *timer_min = temp_timer_min;
                                    else
                                        *timer_min = TIMER_MIN_VALUE_MINUTES;
                                }
                            } else {
                                if ((*temp - 1) >= temp_min) {
                                    is_target_temp_changed = true;
                                    *temp -= 1;
                                }
                            }
                        }
                    } else { // pressed
                        btn_down_press_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
                    }
                } else if ((*btn & (1 << BUTTON_TIMER_FORWARD_STAT)) != (prev_btn & (1 << BUTTON_TIMER_FORWARD_STAT))) { // timer button toggles
                    if ((*btn >> BUTTON_TIMER_FORWARD_STAT) & 0x01) { // unpressed
                        if (!data->is_child_lock_active) {
                            is_timer_change_en = true;
                            is_timer_changed = true; // to update display only
                        }
                    } else { // pressed
                        btn_timer_press_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
                    }
                }
            }
            prev_btn = *btn;
        }

        // update display if ambient temperature changed
        if (*ambient_temp_c != prev_ambient_temp_c) {
            prev_ambient_temp_c = *ambient_temp_c;
            update_display = true;
        }

        // update display if timer value changed
        if (*timer_min != prev_timer_min) {
            printf("timer_min=%d\r\n", *timer_min);
            prev_timer_min = *timer_min;
            update_display = true;
        }

        // normal timer mode display or timer changed display
        if (is_timer_changed) {
            is_timer_changed = false;

            printf("timer=%d\r\n", *timer_min);

            t_timer_changed_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
            display_timer = true;
            update_display = true;

            // update saved timer in flash
            data->last_timer_setting_min = *timer_min;
            set_integer_to_storage(STORAGE_KEY_LAST_TIMER_SETTING, data->last_timer_setting_min);
        } else {
            if (display_timer) {
                if ((xTaskGetTickCount() * portTICK_PERIOD_MS) - t_timer_changed_ms >= DISPLAY_TIMER_DUR_MS) {
                    display_timer = false;
                    update_display = true;
                    is_timer_change_en = false;
                } else {
                    update_display = false;
                }
            }
        }

        if (is_target_temp_changed) {
            is_target_temp_changed = false;

            printf("target temp=%d\r\n", *temp);

            // update the temperature of the other unit
            if (data->settings.temperature_unit == TEMP_UNIT_CELSIUS) {
                data->manual_temperature_fahrenheit = celsius_to_fahr(*temp);
            } else {
                data->manual_temperature_celsius = fahr_to_celsius(*temp);
            }

            // display target temperature
            t_target_temp_changed_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
            display_target_temp = true;
            update_display = true;

            // update target temperature in flash
            set_integer_to_storage(STORAGE_KEY_MANUAL_TEMP_CELSIUS, data->manual_temperature_celsius);
            set_integer_to_storage(STORAGE_KEY_MANUAL_TEMP_FAHRENHEIT, data->manual_temperature_fahrenheit);
        } else {
            if (display_target_temp) {
                if ((xTaskGetTickCount() * portTICK_PERIOD_MS) - t_target_temp_changed_ms >= DISPLAY_TARGET_TEMP_DUR_MS) {
                    display_target_temp = false;
                    update_display = true;
                } else {
                    update_display = false;
                }
            }
        }

        // if target temp is remotely changed, update the display
        if (*target_temp_c != prev_target_temp_c) {
            prev_target_temp_c = *target_temp_c;
            if (display_timer == false) {
                update_display = true;
            }
        }

        // update display
        if (update_display) {
            update_display = false;

            display_clear_screen();

            if (display_timer) {
                display_timer_mode_changed(*timer_min, DISPLAY_COLOR);
            } else if (display_target_temp) {
                display_temperature(*temp, DISPLAY_COLOR);
            } else {
                display_timer_mode_normal(data->settings.temperature_unit == TEMP_UNIT_CELSIUS ? *ambient_temp_c : celsius_to_fahr(*ambient_temp_c), data->settings.temperature_unit == TEMP_UNIT_CELSIUS ? *target_temp_c : *target_temp_f, *timer_min, DISPLAY_COLOR);
            }

            // display connection status indication if connected
            if (data->is_connected)
                display_wifi_icon(DISPLAY_COLOR);

            // display lock icon if child lock is active
            if (data->is_child_lock_active)
                display_child_lock_icon(DISPLAY_COLOR);
        }

        vTaskDelay(1 / portTICK_RATE_MS);
    }

    // exit with display on
    if (screen_off)
       display_on();
}

static void auto_mode_task(app_data_t *data) {
    int *btn = &(data->button_status);
    int prev_btn = *btn;
    app_mode_t *mode = &(data->mode);
    time_t btn_power_press_ms = 0, btn_up_press_ms = 0, btn_down_press_ms = 0;
    bool is_heater_on = false;
    int *ambient_temp_c = &(data->ambient_temperature_celsius), prev_ambient_temp_c = -1;
    int *temp_hysteresis_c = &(data->settings.temperature_hysteresis_celsius);
    int *temp_hysteresis_f = &(data->settings.temperature_hysteresis_fahrenheit);
    int hysteresis_c = 0;
    temp_unit_t *temp_unit = &(data->settings.temperature_unit);

    bool screen_off = false;
    time_t t_screen_on_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
    bool update_display = true;

    int auto_temp_c = -1, auto_temp_f = -1, auto_temp_c_prev= -1;
    int hr = -1, min = -1;
    int t_sched_minute = -1, t_now_minute = -1; // time in minute; hour + minute
    int wday = -1;
    int i = -1, j = -1;

    while(*mode == APP_MODE_AUTO) {
        int sched_past_idx = -1;
        int sched_next_idx = -1;

        wday = clock_get_day_of_week();

        auto_mode_sched_t *sched_week[7 * AUTO_MODE_SCHED_NUM];
        for (i = 0; i < 7 * AUTO_MODE_SCHED_NUM; i++) {
            if ((i == 0) || (i == 1) || (i == 2) || (i == 3) // Sunday
                || (i == 24) || (i == 25) || (i == 26) || (i == 27)) { // Saturday
                sched_week[i] = &sched_weekend[i % AUTO_MODE_SCHED_NUM];
            } else { // weekday
                sched_week[i] = &sched_weekday[i % AUTO_MODE_SCHED_NUM];
            }
        }

        // get time and convert it to minutes
        clock_get_time(&hr, &min, NULL);
//        printf("AUTO: hr=%d min=%d\r\n", hr, min);
        t_now_minute = hr * 60 + min;

        // get past sched
        sched_past_idx = -1;
        // check for the nearest schedule before t_now
        // start with the last sched of the day
        j = wday * AUTO_MODE_SCHED_NUM + AUTO_MODE_SCHED_NUM - 1;
        for (i = 0; i < AUTO_MODE_SCHED_NUM; i++, j--) {
            if (sched_week[j]->en) {
                t_sched_minute = sched_week[j]->hour * 60 + sched_week[j]->minute;
                if ((t_sched_minute == t_now_minute)
                    || (t_sched_minute < t_now_minute)) {
                    sched_past_idx = j;
                    break;
                }
            }
        }

        // if past sched is not found on the current day, search for the first enabled sched from the previous days
        if (sched_past_idx == -1) {
            for (i = 1; i < 7 * AUTO_MODE_SCHED_NUM; i++, j--) {
                if (j < 0) {
                   j = 7 * AUTO_MODE_SCHED_NUM - 1;
                }

                if (sched_week[j]->en) {
                    sched_past_idx = j;
                    break;
                }
            }
        }

//        printf("\r\nsched_past_idx %d wday %d\r\n", sched_past_idx, sched_past_idx / AUTO_MODE_SCHED_NUM);

        // set AUTO temperature
#if AUTO_MODE_INTERPOLATE == 0
        if (sched_past_idx != -1) {
            auto_temp_c = sched_week[sched_past_idx]->temp_c;
            auto_temp_f = sched_week[sched_past_idx]->temp_f;
#else
        // there should be past sched to get next sched
        if (sched_past_idx != -1) {
            // get next sched
            sched_next_idx = -1;
            // check for the nearest schedule after t_now
            // start with the first sched of the day
            j = wday * AUTO_MODE_SCHED_NUM;
            for (i = 0; i < AUTO_MODE_SCHED_NUM; i++, j++) {
                if (sched_week[j]->en) {
                    t_sched_minute = sched_week[j]->hour * 60 + sched_week[j]->minute;
                    if ((t_sched_minute == t_now_minute)
                        || (t_sched_minute > t_now_minute)) {
                        sched_next_idx = j;
                        break;
                    }
                }
            }

            // if next sched is not found on the current day, search for the first enabled sched from the next days
            if (sched_next_idx == -1) {
                for (i = 1; i < 7 * AUTO_MODE_SCHED_NUM; i++, j++) {
                    if (j == 7 * AUTO_MODE_SCHED_NUM) {
                       j = 0;
                    }

                    if (sched_week[j]->en) {
                        sched_next_idx = j;
                        break;
                    }
                }
            }
//            printf("\r\nsched_next_idx %d wday %d\r\n", sched_next_idx, sched_next_idx / AUTO_MODE_SCHED_NUM);
        }

        if ((sched_past_idx != -1) && (sched_next_idx != -1)) { // at least 1 schedule / period is set
            auto_mode_sched_t *sched_past = NULL;
            auto_mode_sched_t *sched_next = NULL;
            int wday_sched_past;
            int wday_sched_next;
            int t_past_minute;
            int temp_past_c;
            int temp_past_f;
            int t_next_minute;
            int temp_next_c;
            int temp_next_f;
            int32_t t_now_past_diff_minute;
            int32_t t_next_past_diff_minute;

            sched_past = sched_week[sched_past_idx];
            sched_next = sched_week[sched_next_idx];
            wday_sched_past = sched_past_idx / AUTO_MODE_SCHED_NUM;
            wday_sched_next = sched_next_idx / AUTO_MODE_SCHED_NUM;

            t_past_minute = sched_past->hour * 60 + sched_past->minute;
            temp_past_c = sched_past->temp_c;
            temp_past_f = sched_past->temp_f;
            t_next_minute = sched_next->hour * 60 + sched_next->minute;
            temp_next_c = sched_next->temp_c;
            temp_next_f = sched_next->temp_f;

//            printf("\r\nt_now_minute %d", t_now_minute);
//            printf("\r\ntemp_past_c %d t_past_minute %d", temp_past_c, t_past_minute);
//            printf("\r\ntemp_next_c %d t_next_minute %d", temp_next_c, t_next_minute);

            t_now_past_diff_minute = t_now_minute - t_past_minute;
            if (wday < wday_sched_past) {
                t_now_past_diff_minute += (wday + 6 - wday_sched_past) * 60 * 24;
            } else {
                t_now_past_diff_minute += (wday - wday_sched_past) * 60 * 24;
            }

            t_next_past_diff_minute = t_next_minute - t_past_minute;
            if (wday_sched_next < wday_sched_past) {
                t_next_past_diff_minute += (wday_sched_next + 6 - wday_sched_past) * 60 * 24;
            } else {
                t_next_past_diff_minute += (wday_sched_next - wday_sched_past) * 60 * 24;
            }

//            printf("\r\nt_now_past_diff_minute %d", t_now_past_diff_minute);
//            printf("\r\nt_next_past_diff_minute %d", t_next_past_diff_minute);

            if (t_next_minute == t_past_minute) {
                auto_temp_c = temp_past_c;
                auto_temp_f = temp_past_f;
            } else {
                if (t_next_past_diff_minute) {
                    auto_temp_c = temp_past_c + t_now_past_diff_minute * (temp_next_c - temp_past_c) / t_next_past_diff_minute;
                    auto_temp_f = temp_past_f + t_now_past_diff_minute * (temp_next_f - temp_past_f) / t_next_past_diff_minute;
                } else {
                    auto_temp_c = temp_past_c;
                    auto_temp_f = temp_past_f;
                }
            }
#endif
        } else { // no schedule / period is set, use set manual temperature
            auto_temp_c = data->manual_temperature_celsius;  
            auto_temp_f = data->manual_temperature_fahrenheit; 
        }

//        printf("\r\nAUTO: auto_temp_c=%d auto_temp_f=%d\r\n", auto_temp_c, auto_temp_f);

        if (auto_temp_c != auto_temp_c_prev) {
            auto_temp_c_prev = auto_temp_c;
            update_display = true;
        }

        // turn off/on the heater based on temperature
        if (*ambient_temp_c < auto_temp_c) {
            if (!is_heater_on) {
                heater_on();
                is_heater_on = true;
                printf("AUTO: heater on ambient=%d target=%d\r\n", *ambient_temp_c, auto_temp_c);
            }
        } else {
            if (*temp_unit == TEMP_UNIT_CELSIUS)
                hysteresis_c = *temp_hysteresis_c;
            else
                hysteresis_c = fahr_to_celsius(*temp_hysteresis_f);

            if (*ambient_temp_c >= (auto_temp_c + hysteresis_c)) {
                if (is_heater_on) {
                    heater_off();
                    is_heater_on = false;
                    printf("AUTO: heater off ambient=%d taget=%d\r\n", *ambient_temp_c, auto_temp_c);
                }
            }
        }

        if (*ambient_temp_c != prev_ambient_temp_c) {
            prev_ambient_temp_c = *ambient_temp_c;
            update_display = true;
        }

        /* button
           - single press BACK/UP/DOWN button to go back to Manual Temperature mode
           - long press POWER button to go to standby mode
           - long press UP and DOWN buttons to activate/deactive child lock
           - long press DOWN button to go to MENU mode
        */
        if (*btn == prev_btn) {
            if (data->display_settings.is_auto_screen_off_en) {
                if (((xTaskGetTickCount() * portTICK_PERIOD_MS) - t_screen_on_ms) >= (data->display_settings.auto_screen_off_delay_sec * 1000)) {
                    display_off();
                    screen_off = true;
                }
            }

            if (!screen_off) {
                if (!((*btn >> BUTTON_POWER_BACK_STAT) & 0x01)) { // power button is pressed
                    int cur_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
                    if ((cur_ms - btn_power_press_ms) >= HEATER_OFF_LONG_PRESS_DUR_MS) {
                        *mode = APP_MODE_STANDBY;
                        btn_power_press_ms = cur_ms;
                    }
                } else if ((!((*btn >> BUTTON_UP_STAT) & 0x01)) && (!((*btn >> BUTTON_DOWN_STAT) & 0x01))) { // both button up or down is pressed
                    int cur_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
                    if (((cur_ms - btn_up_press_ms) >= CHILD_LOCK_LONG_PRESS_DUR_MS)
                        && ((cur_ms - btn_down_press_ms) >= CHILD_LOCK_LONG_PRESS_DUR_MS)) {
                        if (data->settings.is_child_lock_en) {
                            data->is_child_lock_active = !data->is_child_lock_active;
                            printf("is_child_lock_active=%d\r\n", data->is_child_lock_active);

                            // display / remove child lock logo
                            display_child_lock_icon(data->is_child_lock_active ? DISPLAY_COLOR : !DISPLAY_COLOR);
                            btn_up_press_ms = cur_ms;
                            btn_down_press_ms = cur_ms;
                        }
                    }
                } else if (!((*btn >> BUTTON_DOWN_STAT) & 0x01)) { // only button down is pressed
                    int cur_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
                    if ((cur_ms - btn_down_press_ms) >= MENU_MODE_LONG_PRESS_DUR_MS) {
                        if (!data->is_child_lock_active) {
                            *mode = APP_MODE_MENU;
                            data->menu_mode_exit_mode = APP_MODE_AUTO;
                        }
                        btn_down_press_ms = cur_ms;
                    }
                }
            }
        } else { // any of the buttons was pressed
            if (data->display_settings.is_auto_screen_off_en) {
                t_screen_on_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
            }

            if (screen_off) {
                // enable screen only if all buttons are unpressed
                if (((*btn >> BUTTON_POWER_BACK_STAT) & 0x01) && ((*btn >> BUTTON_UP_STAT) & 0x01)
                    && ((*btn >> BUTTON_DOWN_STAT) & 0x01) && ((*btn >> BUTTON_TIMER_FORWARD_STAT) & 0x01)) {
                    screen_off = false;
                    display_on();
                }
            } else {
                if ((*btn & (1 << BUTTON_POWER_BACK_STAT)) != (prev_btn & (1 << BUTTON_POWER_BACK_STAT))) { // power button toggles
                    if ((*btn >> BUTTON_POWER_BACK_STAT) & 0x01) { // unpressed
                        if (!data->is_child_lock_active) {
                            *mode = APP_MODE_MANUAL_TEMPERATURE;
                        }
                    } else {
                        btn_power_press_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
                    }
                } else if ((*btn & (1 << BUTTON_UP_STAT)) != (prev_btn & (1 << BUTTON_UP_STAT))) { // up button toggles
                    if ((*btn >> BUTTON_UP_STAT) & 0x01) { // unpressed
                        if(!data->is_child_lock_active) {
                            *mode = APP_MODE_MANUAL_TEMPERATURE;
                        }
                    } else { // pressed
                        btn_up_press_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
                    }
                } else if ((*btn & (1 << BUTTON_DOWN_STAT)) != (prev_btn & (1 << BUTTON_DOWN_STAT))) { // down button toggles
                    if ((*btn >> BUTTON_DOWN_STAT) & 0x01) { // unpressed
                        if(!data->is_child_lock_active) {
                            *mode = APP_MODE_MANUAL_TEMPERATURE;
                        }
                    } else { // pressed
                        btn_down_press_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
                    }
                }
            }
            prev_btn = *btn;
        }

        // update display
        if (update_display) {
            update_display = false;
            display_clear_screen();
            display_auto_mode(data->settings.temperature_unit == TEMP_UNIT_CELSIUS ? *ambient_temp_c : celsius_to_fahr(*ambient_temp_c),
                data->settings.temperature_unit == TEMP_UNIT_CELSIUS ? auto_temp_c : auto_temp_f, DISPLAY_COLOR);
            // display AUTO icon
            display_auto_icon(DISPLAY_COLOR);
            // display child lock icon if locked
            if (data->is_child_lock_active)
                display_child_lock_icon(DISPLAY_COLOR);
            // display connection status indication if connected
            if (data->is_connected)
                display_wifi_icon(DISPLAY_COLOR);
        }

        vTaskDelay(1 / portTICK_RATE_MS);
    }

    // exit with display on
    if (screen_off)
       display_on();
}

static void menu_mode_task(app_data_t *data) {
    int *btn = &(data->button_status);
    int prev_btn = -1;
    app_mode_t *mode = &(data->mode);
    bool update_display = true;
    uint8_t menu = 0;
    time_t btn_power_press_ms = 0;

    bool screen_off = false;
    time_t t_screen_on_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;

    // display menu navigation screen
    display_clear_screen();
    display_menu_inst(DISPLAY_COLOR);
    // display for MENU_NAVIGATION_SCREEN_DISPLAY_DURATION_MS milliseconds
    vTaskDelay(MENU_NAVIGATION_SCREEN_DISPLAY_DURATION_MS / portTICK_RATE_MS);

    // set prev_btn to current button status
    prev_btn = *btn;

    // menu starts with schedule
    menu = MENU_CALENDAR;

    while(*mode == APP_MODE_MENU) {
        /* button
           - single press BACK button to go back to previous screen if not on the first level
           - single press BACK button to go back to previous mode if on the first level
           - single press UP or DOWN button to navigate the menu
           - single press FORWARD button to enter selected menu
           - long press POWER button to go to standby mode
        */
        if (*btn == prev_btn) {
            if (data->display_settings.is_auto_screen_off_en) {
                if (((xTaskGetTickCount() * portTICK_PERIOD_MS) - t_screen_on_ms) >= (data->display_settings.auto_screen_off_delay_sec * 1000)) {
                    display_off();
                    screen_off = true;
                }
            }

            if (!screen_off) {
                if (!((*btn >> BUTTON_POWER_BACK_STAT) & 0x01)) { // power button is pressed
                    int cur_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
                    if ((cur_ms - btn_power_press_ms) >= HEATER_OFF_LONG_PRESS_DUR_MS) {
                        *mode = APP_MODE_STANDBY;
                        btn_power_press_ms = cur_ms;
                    }
                }
            }
        } else { // any of the buttons was pressed
            if (data->display_settings.is_auto_screen_off_en) {
                t_screen_on_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
            }

            if (screen_off) {
                // enable screen only if all buttons are unpressed
                if (((*btn >> BUTTON_POWER_BACK_STAT) & 0x01) && ((*btn >> BUTTON_UP_STAT) & 0x01)
                    && ((*btn >> BUTTON_DOWN_STAT) & 0x01) && ((*btn >> BUTTON_TIMER_FORWARD_STAT) & 0x01)) {
                    screen_off = false;
                    display_on();
                }
            } else {
                if ((*btn & (1 << BUTTON_POWER_BACK_STAT)) != (prev_btn & (1 << BUTTON_POWER_BACK_STAT))) { // power button toggles
                    if ((*btn >> BUTTON_POWER_BACK_STAT) & 0x01) { // unpressed
                        *mode = data->menu_mode_exit_mode;
                    } else {
                        btn_power_press_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
                    }
                } else if ((*btn & (1 << BUTTON_UP_STAT)) != (prev_btn & (1 << BUTTON_UP_STAT))) { // up button toggles
                    if ((*btn >> BUTTON_UP_STAT) & 0x01) { // unpressed
                        if (menu == MENU_LAST) {
                            menu = MENU_FIRST;
                        } else {
                            menu++;
                        }

                        update_display = true;
                    }
                } else if ((*btn & (1 << BUTTON_DOWN_STAT)) != (prev_btn & (1 << BUTTON_DOWN_STAT))) { // down button toggles
                    if ((*btn >> BUTTON_DOWN_STAT) & 0x01) { // unpressed
                        if (menu == MENU_FIRST) {
                            menu = MENU_LAST;
                        } else {
                            menu--;
                        }

                        update_display = true;
                    }
                } else if ((*btn & (1 << BUTTON_TIMER_FORWARD_STAT)) != (prev_btn & (1 << BUTTON_TIMER_FORWARD_STAT))) { // timer button toggles
                    if ((*btn >> BUTTON_TIMER_FORWARD_STAT) & 0x01) { // unpressed
                        switch (menu) {
/*
                        case MENU_ENERGY:
                            *mode = menu_energy(data);
                            break;
*/
                        case MENU_CALENDAR:
                            *mode = menu_calendar(data);
                            break;
                        case MENU_TIME_DATE:
                            *mode = menu_time_and_date(data);
                            break;
                        case MENU_COMMUNICATIONS:
                            *mode = menu_communications(data);
                            break;
                        case MENU_SETTINGS:
                            *mode = menu_settings(data);
                            break;
                        case MENU_DISPLAY_SETTINGS:
                            *mode = menu_display_settings(data);
                            break;
                        case MENU_UPDATE:
                            *mode = menu_update(data);
                            break;
                        }

                        // reset time of screen on
                        t_screen_on_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
                        update_display = true;
                    }
                }
            }
            prev_btn = *btn;
        }

        // update display
        if (update_display) {
            update_display = false;

            display_clear_screen();
            // display connection status indication if connected
            if (data->is_connected)
                display_wifi_icon(DISPLAY_COLOR);
            switch (menu) {
/*
            case MENU_ENERGY:
                display_menu("Energy", DISPLAY_COLOR, NULL, !DISPLAY_COLOR);
                break;
*/
            case MENU_CALENDAR:
                display_menu("Auto", DISPLAY_COLOR, "Mode", DISPLAY_COLOR);
                break;
            case MENU_TIME_DATE:
                display_menu("Time &", DISPLAY_COLOR, "Date", DISPLAY_COLOR);
                break;
            case MENU_COMMUNICATIONS:
                display_menu("Wi-Fi", DISPLAY_COLOR, NULL, !DISPLAY_COLOR);
                break;
            case MENU_SETTINGS:
                display_menu("Settings", DISPLAY_COLOR, NULL, !DISPLAY_COLOR);
                break;
            case MENU_DISPLAY_SETTINGS:
                display_menu("Display", DISPLAY_COLOR, "Settings", DISPLAY_COLOR);
                break;
            case MENU_UPDATE:
                display_menu("Update", DISPLAY_COLOR, NULL, !DISPLAY_COLOR);
                break;
            }
        }

        vTaskDelay(1 / portTICK_RATE_MS);
    }

    // exit with display on
    if (screen_off)
       display_on();
}

static void button_up_cb(int level) {
    uint8_t prev = (app_data->button_status >> BUTTON_UP_STAT) & 0x01;

    if (prev != level) {
        printf("ui : UP\r\n");

        if (level)
            app_data->button_status |= 1 << BUTTON_UP_STAT;
        else
            app_data->button_status &= ~(1 << BUTTON_UP_STAT);
    }
}

static void button_down_cb(int level) {
    uint8_t prev = (app_data->button_status >> BUTTON_DOWN_STAT) & 0x01;

    if (prev != level) {
        printf("ui : D0WN\r\n");

        if (level)
            app_data->button_status |= 1 << BUTTON_DOWN_STAT;
        else
            app_data->button_status &= ~(1 << BUTTON_DOWN_STAT);
    }
}

static void button_power_back_cb(int level) {
    uint8_t prev = (app_data->button_status >> BUTTON_POWER_BACK_STAT) & 0x01;

    if (prev != level) {
        printf("ui : POWER_BACK\r\n");

        if (level)
            app_data->button_status |= 1 << BUTTON_POWER_BACK_STAT;
        else
            app_data->button_status &= ~(1 << BUTTON_POWER_BACK_STAT);
    }
}

static void button_timer_forward_cb(int level) {
    uint8_t prev = (app_data->button_status >> BUTTON_TIMER_FORWARD_STAT) & 0x01;

    if (prev != level) {
        printf("ui : TIMER_FORWARD\r\n");

        if (level)
            app_data->button_status |= 1 << BUTTON_TIMER_FORWARD_STAT;
        else
            app_data->button_status &= ~(1 << BUTTON_TIMER_FORWARD_STAT);
    }
}

/*
static app_mode_t menu_energy(app_data_t *data) {
    int *btn = &(data->button_status);
    int prev_btn = *btn;

    // when changing mode, this task should be completed before starting the next mode
    bool exit = false;
    app_mode_t next_mode = data->mode;

    uint8_t m_energy = MENU_ENERGY_DAY;
    bool update_display = true;
    time_t btn_power_press_ms = 0;

    bool screen_off = false;
    time_t t_screen_on_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;

    while (!exit) {
        if (*btn == prev_btn) {
            if (data->display_settings.is_auto_screen_off_en) {
                if (((xTaskGetTickCount() * portTICK_PERIOD_MS) - t_screen_on_ms) >= (data->display_settings.auto_screen_off_delay_sec * 1000)) {
                    display_off();
                    screen_off = true;
                }
            }

            if (!screen_off) {
                if (!((*btn >> BUTTON_POWER_BACK_STAT) & 0x01)) { // power button is pressed
                    int cur_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
                    if ((cur_ms - btn_power_press_ms) >= HEATER_OFF_LONG_PRESS_DUR_MS) {
                        next_mode = APP_MODE_STANDBY;
                        exit = true;
                        btn_power_press_ms = cur_ms;
                    }
                }
            }
        } else { // any of the buttons was pressed
            if (data->display_settings.is_auto_screen_off_en) {
                t_screen_on_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
            }

            if (screen_off) {
                // enable screen only if all buttons are unpressed
                if (((*btn >> BUTTON_POWER_BACK_STAT) & 0x01) && ((*btn >> BUTTON_UP_STAT) & 0x01)
                    && ((*btn >> BUTTON_DOWN_STAT) & 0x01) && ((*btn >> BUTTON_TIMER_FORWARD_STAT) & 0x01)) {
                    screen_off = false;
                    display_on();
                }
            } else {
                if ((*btn & (1 << BUTTON_POWER_BACK_STAT)) != (prev_btn & (1 << BUTTON_POWER_BACK_STAT))) { // power button toggles
                    if ((*btn >> BUTTON_POWER_BACK_STAT) & 0x01) { // unpressed
                        switch (m_energy) {
                        case MENU_ENERGY_DAY:
                        case MENU_ENERGY_WEEK:
                        case MENU_ENERGY_MONTH:
                            exit = true;
                            break;
                        case MENU_ENERGY_DAY_VAL:
                            m_energy = MENU_ENERGY_DAY;
                            break;
                        case MENU_ENERGY_WEEK_VAL:
                            m_energy = MENU_ENERGY_WEEK;
                            break;
                        case MENU_ENERGY_MONTH_VAL:
                            m_energy = MENU_ENERGY_MONTH;
                            break;
                        }

                        update_display = true;
                    } else {
                        btn_power_press_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
                    }
                } else if ((*btn & (1 << BUTTON_UP_STAT)) != (prev_btn & (1 << BUTTON_UP_STAT))) { // up button toggles
                    if ((*btn >> BUTTON_UP_STAT) & 0x01) { // unpressed
                        switch (m_energy) {
                        case MENU_ENERGY_DAY:
                            m_energy = MENU_ENERGY_WEEK;
                            break;
                        case MENU_ENERGY_WEEK:
                            m_energy = MENU_ENERGY_MONTH;
                            break;
                        case MENU_ENERGY_MONTH:
                            m_energy = MENU_ENERGY_DAY;
                            break;
                        }

                        update_display = true;
                    }
                } else if ((*btn & (1 << BUTTON_DOWN_STAT)) != (prev_btn & (1 << BUTTON_DOWN_STAT))) { // down button toggles
                    if ((*btn >> BUTTON_DOWN_STAT) & 0x01) { // unpressed
                        switch (m_energy) {
                        case MENU_ENERGY_DAY:
                            m_energy = MENU_ENERGY_MONTH;
                            break;
                        case MENU_ENERGY_WEEK:
                            m_energy = MENU_ENERGY_DAY;
                            break;
                        case MENU_ENERGY_MONTH:
                            m_energy = MENU_ENERGY_WEEK;
                            break;
                        }

                        update_display = true;
                    }
                } else if ((*btn & (1 << BUTTON_TIMER_FORWARD_STAT)) != (prev_btn & (1 << BUTTON_TIMER_FORWARD_STAT))) { // timer button toggles
                    if ((*btn >> BUTTON_TIMER_FORWARD_STAT) & 0x01) { // unpressed
                        switch (m_energy) {
                        case MENU_ENERGY_DAY:
                            m_energy = MENU_ENERGY_DAY_VAL;
                            break;
                        case MENU_ENERGY_WEEK:
                            m_energy = MENU_ENERGY_WEEK_VAL;
                            break;
                        case MENU_ENERGY_MONTH:
                            m_energy = MENU_ENERGY_MONTH_VAL;
                            break;
                        }

                        update_display = true;
                    }
                }
            }
            prev_btn = *btn;
        }

        // update the display
        if (update_display) {
            printf("m_energy=%d\r\n", m_energy);

            display_clear_screen();
            // display connection status indication if connected
            if (data->is_connected)
                display_wifi_icon(DISPLAY_COLOR);
            switch (m_energy) {
            case MENU_ENERGY_DAY:
                display_menu("Day", DISPLAY_COLOR, NULL, !DISPLAY_COLOR);
                break;
            case MENU_ENERGY_WEEK:
                display_menu("Week", DISPLAY_COLOR, NULL, !DISPLAY_COLOR);
                break;
            case MENU_ENERGY_MONTH:
                display_menu("Month", DISPLAY_COLOR, NULL, !DISPLAY_COLOR);
                break;
            case MENU_ENERGY_DAY_VAL:
                // TODO: display real power consumption of the day
                display_energy(0, ENERGY_UNIT_STR, DISPLAY_COLOR);
                break;
            case MENU_ENERGY_WEEK_VAL:
                // TODO: display real power consumption of the week
                display_energy(0, ENERGY_UNIT_STR, DISPLAY_COLOR);
                break;
            case MENU_ENERGY_MONTH_VAL:
                // TODO: display real power consumption of the month
                display_energy(0, ENERGY_UNIT_STR, DISPLAY_COLOR);
                break;
            }

            update_display = false;
        }

        vTaskDelay(1 / portTICK_RATE_MS);
    }

    // exit with display on
    if (screen_off)
       display_on();

    // return new mode
    return next_mode;
}
*/

static app_mode_t menu_calendar(app_data_t *data) {
    int *btn = &(data->button_status);
    int prev_btn = *btn;

    // when changing mode, this task should be completed before starting the next mode
    bool exit = false;
    app_mode_t next_mode = data->mode;

#if 1 

    uint8_t m_cal = MENU_DISPLAY_SETTINGS_BRIGHTNESS;
    bool update_display = true;
    time_t btn_power_press_ms = 0;

    bool screen_off = false;
    time_t t_screen_on_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;

    auto_mode_sched_t *sched = NULL;
    uint8_t sched_num = 0;
    char sched_num_str[11];
    bool weekend = false; // true if weekend, false if weekday
    bool is_sched_changed = false;

    uint8_t *temp = NULL, *temp_c = NULL, *temp_f = NULL;
    uint8_t temp_max = data->settings.temperature_unit == TEMP_UNIT_CELSIUS ? TEMPERATURE_CELSIUS_VAL_MAX : TEMPERATURE_FAHRENHEIT_VAL_MAX;
    uint8_t temp_min = data->settings.temperature_unit == TEMP_UNIT_CELSIUS ? TEMPERATURE_CELSIUS_VAL_MIN : TEMPERATURE_FAHRENHEIT_VAL_MIN;

    while (!exit) {
        if (*btn == prev_btn) {
            if (data->display_settings.is_auto_screen_off_en) {
                if (((xTaskGetTickCount() * portTICK_PERIOD_MS) - t_screen_on_ms) >= (data->display_settings.auto_screen_off_delay_sec * 1000)) {
                    display_off();
                    screen_off = true;
                }
            }

            if (!screen_off) {
                if (!((*btn >> BUTTON_POWER_BACK_STAT) & 0x01)) { // power button is pressed
                    int cur_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
                    if ((cur_ms - btn_power_press_ms) >= HEATER_OFF_LONG_PRESS_DUR_MS) {
                        next_mode = APP_MODE_STANDBY;
                        exit = true;
                        btn_power_press_ms = cur_ms;
                    }
                }
            }
        } else {
            if (data->display_settings.is_auto_screen_off_en) {
                t_screen_on_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
            }

            if (screen_off) {
                // enable screen only if all buttons are unpressed
                if (((*btn >> BUTTON_POWER_BACK_STAT) & 0x01) && ((*btn >> BUTTON_UP_STAT) & 0x01)
                    && ((*btn >> BUTTON_DOWN_STAT) & 0x01) && ((*btn >> BUTTON_TIMER_FORWARD_STAT) & 0x01)) {
                    screen_off = false;
                    display_on();
                }
            } else {
                if ((*btn & (1 << BUTTON_POWER_BACK_STAT)) != (prev_btn & (1 << BUTTON_POWER_BACK_STAT))) { // power button toggles
                    if ((*btn >> BUTTON_POWER_BACK_STAT) & 0x01) { // unpressed
                        switch (m_cal) {
                        case MENU_CALENDAR_WEEKTYPE:
                            exit = true;
                            break;
                        case MENU_CALENDAR_SCHED:
                            m_cal = MENU_CALENDAR_WEEKTYPE;
                            break;
                        case MENU_CALENDAR_SCHED_EN:
                        case MENU_CALENDAR_SCHED_TIME:
                        case MENU_CALENDAR_SCHED_TEMP:
                            m_cal = MENU_CALENDAR_SCHED;
                            break;
                        case MENU_CALENDAR_SCHED_EN_STAT:
                            m_cal = MENU_CALENDAR_SCHED_EN;
                            break;
                        case MENU_CALENDAR_SCHED_TIME_HOUR:
                        case MENU_CALENDAR_SCHED_TIME_MINUTE:
                            m_cal = MENU_CALENDAR_SCHED_TIME;
                            break;
                        case MENU_CALENDAR_SCHED_TEMP_CHANGE:
                            m_cal = MENU_CALENDAR_SCHED_TEMP;
                            break;
                        case MENU_CALENDAR_SCHED_TIME_HOUR_CHANGE:
                            m_cal = MENU_CALENDAR_SCHED_TIME_HOUR;
                            break;
                        case MENU_CALENDAR_SCHED_TIME_MINUTE_CHANGE:
                            m_cal = MENU_CALENDAR_SCHED_TIME_MINUTE;
                            break;
                        }

                        update_display = true;
                    } else { // pressed
                        btn_power_press_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
                    }
                } else if ((*btn & (1 << BUTTON_UP_STAT)) != (prev_btn & (1 << BUTTON_UP_STAT))) { // up button toggles
                    if ((*btn >> BUTTON_UP_STAT) & 0x01) { // unpressed
                        switch (m_cal) {
                        case MENU_CALENDAR_WEEKTYPE:
                            weekend = !weekend;
                            break;
                        case MENU_CALENDAR_SCHED:
                            if (sched_num < (AUTO_MODE_SCHED_NUM - 1))
                                ++sched_num;
                            else
                                sched_num = 0;
                            break;
                        case MENU_CALENDAR_SCHED_EN:
                            m_cal = MENU_CALENDAR_SCHED_TIME;
                            break;
                        case MENU_CALENDAR_SCHED_TIME:
                            m_cal = MENU_CALENDAR_SCHED_TEMP;
                            break;
                        case MENU_CALENDAR_SCHED_TEMP:
                            m_cal = MENU_CALENDAR_SCHED_EN;
                            break;
                        case MENU_CALENDAR_SCHED_EN_STAT:
                            sched[sched_num].en = !sched[sched_num].en;
                            break;
                        case MENU_CALENDAR_SCHED_TIME_HOUR:
                            m_cal = MENU_CALENDAR_SCHED_TIME_MINUTE;
                            break;
                        case MENU_CALENDAR_SCHED_TIME_MINUTE:
                            m_cal = MENU_CALENDAR_SCHED_TIME_HOUR;
                            break;
                        case MENU_CALENDAR_SCHED_TEMP_CHANGE:
                            if (*temp < temp_max) {
                                is_sched_changed = true;
                                *temp += 1;

                                // update value of the other unit
                                if (data->settings.temperature_unit == TEMP_UNIT_CELSIUS)
                                    *temp_f = celsius_to_fahr(*temp_c);
                                else
                                    *temp_c = fahr_to_celsius(*temp_f);
                            }
                            break;
                        case MENU_CALENDAR_SCHED_TIME_HOUR_CHANGE:
                            if (sched[sched_num].hour < 23) {
                                is_sched_changed = true;
                                sched[sched_num].hour++;
                            }
                            break;
                        case MENU_CALENDAR_SCHED_TIME_MINUTE_CHANGE:
                            if (sched[sched_num].minute < 59) {
                                is_sched_changed = true;
                                sched[sched_num].minute++;
                            }
                            break;
                        }

                        update_display = true;
                    }
                } else if ((*btn & (1 << BUTTON_DOWN_STAT)) != (prev_btn & (1 << BUTTON_DOWN_STAT))) { // down button toggles
                    if ((*btn >> BUTTON_DOWN_STAT) & 0x01) { // unpressed
                        switch (m_cal) {
                        case MENU_CALENDAR_WEEKTYPE:
                            weekend = !weekend;
                            break;
                        case MENU_CALENDAR_SCHED:
                            if (sched_num > 0)
                                --sched_num;
                            else
                                sched_num = AUTO_MODE_SCHED_NUM - 1;
                            break;
                        case MENU_CALENDAR_SCHED_EN:
                            m_cal = MENU_CALENDAR_SCHED_TEMP;
                            break;
                        case MENU_CALENDAR_SCHED_TIME:
                            m_cal = MENU_CALENDAR_SCHED_EN;
                            break;
                        case MENU_CALENDAR_SCHED_TEMP:
                            m_cal = MENU_CALENDAR_SCHED_TIME;
                            break;
                        case MENU_CALENDAR_SCHED_EN_STAT:
                            sched[sched_num].en = !sched[sched_num].en;
                            break;
                        case MENU_CALENDAR_SCHED_TIME_HOUR:
                            m_cal = MENU_CALENDAR_SCHED_TIME_MINUTE;
                            break;
                        case MENU_CALENDAR_SCHED_TIME_MINUTE:
                            m_cal = MENU_CALENDAR_SCHED_TIME_HOUR;
                            break;
                        case MENU_CALENDAR_SCHED_TEMP_CHANGE:
                            if (*temp > temp_min) {
                                is_sched_changed = true;
                                *temp -= 1;

                                // update value of the other unit
                                if (data->settings.temperature_unit == TEMP_UNIT_CELSIUS)
                                    *temp_f = celsius_to_fahr(*temp_c);
                                else
                                    *temp_c = fahr_to_celsius(*temp_f);
                            }
                            break;
                        case MENU_CALENDAR_SCHED_TIME_HOUR_CHANGE:
                            if (sched[sched_num].hour > 0) {
                                is_sched_changed = true;
                                sched[sched_num].hour--;
                            }
                            break;
                        case MENU_CALENDAR_SCHED_TIME_MINUTE_CHANGE:
                            if (sched[sched_num].minute > 0) {
                                is_sched_changed = true;
                                sched[sched_num].minute--;
                            }
                            break;
                        }

                        update_display = true;
                    }
                } else if ((*btn & (1 << BUTTON_TIMER_FORWARD_STAT)) != (prev_btn & (1 << BUTTON_TIMER_FORWARD_STAT))) { // timer button toggles
                    if ((*btn >> BUTTON_TIMER_FORWARD_STAT) & 0x01) { // unpressed
                        switch (m_cal) {
                        case MENU_CALENDAR_WEEKTYPE:
                            sched_num = 0;

                            // set sched
                            if (weekend)
                                sched = sched_weekend;
                            else
                                sched = sched_weekday;

                            m_cal = MENU_CALENDAR_SCHED;
                            break;
                        case MENU_CALENDAR_SCHED:
                            // get target temp
                            temp_c = &(sched[sched_num].temp_c);
                            temp_f = &(sched[sched_num].temp_f);
                            temp = data->settings.temperature_unit == TEMP_UNIT_CELSIUS ? temp_c : temp_f;
                            m_cal = MENU_CALENDAR_SCHED_EN;
                            break;
                        case MENU_CALENDAR_SCHED_EN:
                            m_cal = MENU_CALENDAR_SCHED_EN_STAT;
                            break;
                        case MENU_CALENDAR_SCHED_TIME:
                            m_cal = MENU_CALENDAR_SCHED_TIME_HOUR;
                            break;
                        case MENU_CALENDAR_SCHED_TEMP:
                            m_cal = MENU_CALENDAR_SCHED_TEMP_CHANGE;
                            break;
                        case MENU_CALENDAR_SCHED_EN_STAT:
                            // do nothing
                            break;
                        case MENU_CALENDAR_SCHED_TIME_HOUR:
                            m_cal = MENU_CALENDAR_SCHED_TIME_HOUR_CHANGE;
                            break;
                        case MENU_CALENDAR_SCHED_TIME_MINUTE:
                            m_cal = MENU_CALENDAR_SCHED_TIME_MINUTE_CHANGE;
                            break;
                        case MENU_CALENDAR_SCHED_TEMP_CHANGE:
                            // do nothing
                            break;
                        case MENU_CALENDAR_SCHED_TIME_HOUR_CHANGE:
                            // do nothing
                            break;
                        case MENU_CALENDAR_SCHED_TIME_MINUTE_CHANGE:
                            // do nothing
                            break;
                        }

                        update_display = true;
                    }
                }
            }
            prev_btn = *btn;
        }

        // update display
        if (update_display) {
            update_display = false;

            display_clear_screen();
            // display connection status indication if connected
            if (data->is_connected)
                display_wifi_icon(DISPLAY_COLOR);
            switch (m_cal) {
            case MENU_CALENDAR_WEEKTYPE:
                display_menu(weekend ? "Weekend" : "Weekday", DISPLAY_COLOR, NULL, !DISPLAY_COLOR);
                break;
            case MENU_CALENDAR_SCHED:
                if (sched_num == 0) {
                    sprintf(sched_num_str, "Wake");
                } else if (sched_num == 1) {
                    sprintf(sched_num_str, "Leave");
                } else if (sched_num == 2) {
                    sprintf(sched_num_str, "Return");
                } else if (sched_num == 3) {
                    sprintf(sched_num_str, "Sleep");
                }
//                sprintf(sched_num_str, "Sched %d", sched_num + 1);
                display_menu(sched_num_str, DISPLAY_COLOR, NULL, !DISPLAY_COLOR);
                break;
            case MENU_CALENDAR_SCHED_EN:
                display_menu("Enable", DISPLAY_COLOR, sched_num_str, DISPLAY_COLOR);
                break;
            case MENU_CALENDAR_SCHED_TIME:
                display_menu("Time", DISPLAY_COLOR, NULL, !DISPLAY_COLOR);
                break;
            case MENU_CALENDAR_SCHED_TEMP:
                display_menu("Temp", DISPLAY_COLOR, NULL, !DISPLAY_COLOR);
                break;
            case MENU_CALENDAR_SCHED_EN_STAT:
                display_menu(sched[sched_num].en ? "Enabled" :  "Disabled", DISPLAY_COLOR, NULL, !DISPLAY_COLOR);
                break;
            case MENU_CALENDAR_SCHED_TIME_HOUR:
                display_menu("Hour", DISPLAY_COLOR, NULL, !DISPLAY_COLOR);
                break;
            case MENU_CALENDAR_SCHED_TIME_MINUTE:
                display_menu("Minute", DISPLAY_COLOR, NULL, !DISPLAY_COLOR);
                break;
            case MENU_CALENDAR_SCHED_TEMP_CHANGE:
                display_temperature(*temp, DISPLAY_COLOR);
                break;
            case MENU_CALENDAR_SCHED_TIME_HOUR_CHANGE:
                display_hour(sched[sched_num].hour, DISPLAY_COLOR);
                break;
            case MENU_CALENDAR_SCHED_TIME_MINUTE_CHANGE:
                display_minute(sched[sched_num].minute, DISPLAY_COLOR);
                break;
            }

        }

        vTaskDelay(1 / portTICK_RATE_MS);
    }

    // exit with display on
    if (screen_off)
       display_on();

    // save schedule if changed
    if (is_sched_changed) {
        set_data_to_storage(STORAGE_KEY_SCHED_WEEKDAY, (void *) sched_weekday, sizeof(sched_weekday));
        set_data_to_storage(STORAGE_KEY_SCHED_WEEKEND, (void *) sched_weekend, sizeof(sched_weekend));
    }

#endif
    // return new mode
    return next_mode;
}


static app_mode_t menu_time_and_date(app_data_t *data) {
    int *btn = &(data->button_status);
    int prev_btn = *btn;

    // when changing mode, this task should be completed before starting the next mode
    bool exit = false;
    app_mode_t next_mode = data->mode;

    uint8_t m_timedate = MENU_TIME_AND_DATE_AUTO;
    bool update_display = true;
    int year, month, day, hour, minute;
    time_t btn_power_press_ms = 0;

    bool screen_off = false;
    time_t t_screen_on_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;

    bool display_date_change_en = false;
    bool display_time_change_en = false;

    bool *is_auto_time_date_en = &(data->is_auto_time_date_en);
    bool prev_is_auto_time_date_en = *is_auto_time_date_en;

    int *timezone_offset_idx = &(data->timezone_offset_idx);
    int prev_timezone_offset_idx = *timezone_offset_idx;

    clock_get_date_and_time(&year, &month, &day, &hour, &minute, NULL);

    while (!exit) {
        if (*btn == prev_btn) {
            if (data->display_settings.is_auto_screen_off_en) {
                if (((xTaskGetTickCount() * portTICK_PERIOD_MS) - t_screen_on_ms) >= (data->display_settings.auto_screen_off_delay_sec * 1000)) {
                    display_off();
                    screen_off = true;
                }
            }

            if (!screen_off) {
                if (!((*btn >> BUTTON_POWER_BACK_STAT) & 0x01)) { // power button is pressed
                    int cur_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
                    if ((cur_ms - btn_power_press_ms) >= HEATER_OFF_LONG_PRESS_DUR_MS) {
                        next_mode = APP_MODE_STANDBY;
                        exit = true;
                        btn_power_press_ms = cur_ms;
                    }
                }
            }
        } else { // any of the buttons was pressed
            if (data->display_settings.is_auto_screen_off_en) {
                t_screen_on_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
            }

            if (screen_off) {
                // enable screen only if all buttons are unpressed
                if (((*btn >> BUTTON_POWER_BACK_STAT) & 0x01) && ((*btn >> BUTTON_UP_STAT) & 0x01)
                    && ((*btn >> BUTTON_DOWN_STAT) & 0x01) && ((*btn >> BUTTON_TIMER_FORWARD_STAT) & 0x01)) {
                    screen_off = false;
                    display_on();
                }
            } else {
                if ((*btn & (1 << BUTTON_POWER_BACK_STAT)) != (prev_btn & (1 << BUTTON_POWER_BACK_STAT))) { // power button toggles
                    if ((*btn >> BUTTON_POWER_BACK_STAT) & 0x01) { // unpressed
                        switch (m_timedate) {
                        case MENU_TIME_AND_DATE_AUTO:
                        case MENU_TIME_AND_DATE_MANUAL:
                        case MENU_TIME_AND_DATE_TIMEZONE_OFFSET:
                            exit = true;
                            break;
                        case MENU_TIME_AND_DATE_AUTO_EN:
                            m_timedate = MENU_TIME_AND_DATE_AUTO;
                            break;
                        case MENU_TIME_AND_DATE_MANUAL_DATE:
                        case MENU_TIME_AND_DATE_MANUAL_TIME:
                            m_timedate = MENU_TIME_AND_DATE_MANUAL;
                            break;
                        case MENU_TIME_AND_DATE_TIMEZONE_OFFSET_SET:
                            m_timedate = MENU_TIME_AND_DATE_TIMEZONE_OFFSET;
                            break;
                        case MENU_TIME_AND_DATE_MANUAL_DATE_YEAR:
                        case MENU_TIME_AND_DATE_MANUAL_DATE_MONTH:
                        case MENU_TIME_AND_DATE_MANUAL_DATE_DAY:
                            m_timedate = MENU_TIME_AND_DATE_MANUAL_DATE;
                            break;
                        case MENU_TIME_AND_DATE_MANUAL_TIME_HOUR:
                        case MENU_TIME_AND_DATE_MANUAL_TIME_MINUTE:
                            m_timedate = MENU_TIME_AND_DATE_MANUAL_TIME;
                            break;
                        case MENU_TIME_AND_DATE_MANUAL_DATE_YEAR_CHANGE:
                            m_timedate = MENU_TIME_AND_DATE_MANUAL_DATE_YEAR;
                            display_date_change_en = true;
                            break;
                        case MENU_TIME_AND_DATE_MANUAL_DATE_MONTH_CHANGE:
                            m_timedate = MENU_TIME_AND_DATE_MANUAL_DATE_MONTH;
                            display_date_change_en = true;
                            break;
                        case MENU_TIME_AND_DATE_MANUAL_DATE_DAY_CHANGE:
                            m_timedate = MENU_TIME_AND_DATE_MANUAL_DATE_DAY;
                            display_date_change_en = true;
                            break;
                        case MENU_TIME_AND_DATE_MANUAL_TIME_HOUR_CHANGE:
                            m_timedate = MENU_TIME_AND_DATE_MANUAL_TIME_HOUR;
                            display_time_change_en = true;
                            break;
                        case MENU_TIME_AND_DATE_MANUAL_TIME_MINUTE_CHANGE:
                            m_timedate = MENU_TIME_AND_DATE_MANUAL_TIME_MINUTE;
                            display_time_change_en = true;
                            break;
                        }

                        update_display = true;
                    } else {
                        btn_power_press_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
                    }
                } else if ((*btn & (1 << BUTTON_UP_STAT)) != (prev_btn & (1 << BUTTON_UP_STAT))) { // up button toggles
                    if ((*btn >> BUTTON_UP_STAT) & 0x01) { // unpressed
                        switch (m_timedate) {
                        case MENU_TIME_AND_DATE_AUTO:
                            m_timedate = MENU_TIME_AND_DATE_MANUAL;
                            break;
                        case MENU_TIME_AND_DATE_MANUAL:
                            m_timedate = MENU_TIME_AND_DATE_TIMEZONE_OFFSET;
                            break;
                        case MENU_TIME_AND_DATE_TIMEZONE_OFFSET:
                            m_timedate = MENU_TIME_AND_DATE_AUTO;
                            break;
                        case MENU_TIME_AND_DATE_AUTO_EN:
                            // toggle to enable <--> disable
                            *is_auto_time_date_en = !(*is_auto_time_date_en);
                            break;
                        case MENU_TIME_AND_DATE_MANUAL_DATE:
                            m_timedate = MENU_TIME_AND_DATE_MANUAL_TIME;
                            break;
                        case MENU_TIME_AND_DATE_MANUAL_TIME:
                            m_timedate = MENU_TIME_AND_DATE_MANUAL_DATE;
                            break;
                        case MENU_TIME_AND_DATE_TIMEZONE_OFFSET_SET:
			    if (*timezone_offset_idx < (TIMEZONE_OFFSET_LIST_SIZE - 1))
                                *timezone_offset_idx += 1;
                            break;
                        case MENU_TIME_AND_DATE_MANUAL_DATE_YEAR:
                            m_timedate = MENU_TIME_AND_DATE_MANUAL_DATE_MONTH;
                            break;
                        case MENU_TIME_AND_DATE_MANUAL_DATE_MONTH:
                            m_timedate = MENU_TIME_AND_DATE_MANUAL_DATE_DAY;
                            break;
                        case MENU_TIME_AND_DATE_MANUAL_DATE_DAY:
                            m_timedate = MENU_TIME_AND_DATE_MANUAL_DATE_YEAR;
                            break;
                        case MENU_TIME_AND_DATE_MANUAL_TIME_HOUR:
                            m_timedate = MENU_TIME_AND_DATE_MANUAL_TIME_MINUTE;
                            break;
                        case MENU_TIME_AND_DATE_MANUAL_TIME_MINUTE:
                            m_timedate = MENU_TIME_AND_DATE_MANUAL_TIME_HOUR;
                            break;
                        case MENU_TIME_AND_DATE_MANUAL_DATE_YEAR_CHANGE:
                            // increment year
                            clock_get_date(&year, NULL, NULL);
                            clock_set_year(++year);
                            break;
                        case MENU_TIME_AND_DATE_MANUAL_DATE_MONTH_CHANGE:
                            // increment month
                            clock_get_date(NULL, &month, NULL);
                            clock_set_month(++month);
                            break;
                        case MENU_TIME_AND_DATE_MANUAL_DATE_DAY_CHANGE:
                            // increment day
                            clock_get_date(NULL, NULL, &day);
                            clock_set_day(++day);
                            break;
                        case MENU_TIME_AND_DATE_MANUAL_TIME_HOUR_CHANGE:
                            // increment hour
                            clock_get_time(&hour, NULL, NULL);
                            clock_set_hour(++hour);
                            break;
                        case MENU_TIME_AND_DATE_MANUAL_TIME_MINUTE_CHANGE:
                            // increment minute
                            clock_get_time(NULL, &minute, NULL);
                            clock_set_minute(++minute);
                            break;
                        }

                        update_display = true;
                    }
                } else if ((*btn & (1 << BUTTON_DOWN_STAT)) != (prev_btn & (1 << BUTTON_DOWN_STAT))) { // down button toggles
                    if ((*btn >> BUTTON_DOWN_STAT) & 0x01) { // unpressed
                        switch (m_timedate) {
                        case MENU_TIME_AND_DATE_AUTO:
                            m_timedate = MENU_TIME_AND_DATE_TIMEZONE_OFFSET;
                            break;
                        case MENU_TIME_AND_DATE_MANUAL:
                            m_timedate = MENU_TIME_AND_DATE_AUTO;
                            break;
                        case MENU_TIME_AND_DATE_TIMEZONE_OFFSET:
                            m_timedate = MENU_TIME_AND_DATE_MANUAL;
                            break;
                        case MENU_TIME_AND_DATE_AUTO_EN:
                            // toggle to enable <--> disable
                            *is_auto_time_date_en = !(*is_auto_time_date_en);
                            break;
                        case MENU_TIME_AND_DATE_MANUAL_DATE:
                            m_timedate = MENU_TIME_AND_DATE_MANUAL_TIME;
                            break;
                        case MENU_TIME_AND_DATE_MANUAL_TIME:
                            m_timedate = MENU_TIME_AND_DATE_MANUAL_DATE;
                            break;
                        case MENU_TIME_AND_DATE_TIMEZONE_OFFSET_SET:
                            if (*timezone_offset_idx > 0)
                                *timezone_offset_idx -= 1;
                            break;
                        case MENU_TIME_AND_DATE_MANUAL_DATE_YEAR:
                            m_timedate = MENU_TIME_AND_DATE_MANUAL_DATE_DAY;
                            break;
                        case MENU_TIME_AND_DATE_MANUAL_DATE_MONTH:
                            m_timedate = MENU_TIME_AND_DATE_MANUAL_DATE_YEAR;
                            break;
                        case MENU_TIME_AND_DATE_MANUAL_DATE_DAY:
                            m_timedate = MENU_TIME_AND_DATE_MANUAL_DATE_MONTH;
                            break;
                        case MENU_TIME_AND_DATE_MANUAL_TIME_HOUR:
                            m_timedate = MENU_TIME_AND_DATE_MANUAL_TIME_MINUTE;
                            break;
                        case MENU_TIME_AND_DATE_MANUAL_TIME_MINUTE:
                            m_timedate = MENU_TIME_AND_DATE_MANUAL_TIME_HOUR;
                            break;
                        case MENU_TIME_AND_DATE_MANUAL_DATE_YEAR_CHANGE:
                            // decrement year
                            clock_get_date(&year, NULL, NULL);
                            if ((year - 1) >= MANUFACTURING_YEAR)
                                clock_set_year(--year);
                            break;
                        case MENU_TIME_AND_DATE_MANUAL_DATE_MONTH_CHANGE:
                            // decrement month
                            clock_get_date(NULL, &month, NULL);
                            clock_set_month(--month);
                            break;
                        case MENU_TIME_AND_DATE_MANUAL_DATE_DAY_CHANGE:
                            // decrement day
                            clock_get_date(NULL, NULL, &day);
                            clock_set_day(--day);
                            break;
                        case MENU_TIME_AND_DATE_MANUAL_TIME_HOUR_CHANGE:
                            // decrement hour
                            clock_get_time(&hour, NULL, NULL);
                            clock_set_hour(--hour);
                            break;
                        case MENU_TIME_AND_DATE_MANUAL_TIME_MINUTE_CHANGE:
                            // decrement minute
                            clock_get_time(NULL, &minute, NULL);
                            clock_set_minute(--minute);
                            break;
                        }

                        update_display = true;
                    }
                } else if ((*btn & (1 << BUTTON_TIMER_FORWARD_STAT)) != (prev_btn & (1 << BUTTON_TIMER_FORWARD_STAT))) { // timer button toggles
                    if ((*btn >> BUTTON_TIMER_FORWARD_STAT) & 0x01) { // unpressed
                        switch (m_timedate) {
                        case MENU_TIME_AND_DATE_AUTO:
                            m_timedate = MENU_TIME_AND_DATE_AUTO_EN;
                            break;
                        case MENU_TIME_AND_DATE_MANUAL:
                            m_timedate = MENU_TIME_AND_DATE_MANUAL_DATE;
                            break;
                        case MENU_TIME_AND_DATE_TIMEZONE_OFFSET:
                            m_timedate = MENU_TIME_AND_DATE_TIMEZONE_OFFSET_SET;
                            break;
                        case MENU_TIME_AND_DATE_MANUAL_DATE:
                            m_timedate = MENU_TIME_AND_DATE_MANUAL_DATE_YEAR;
                            break;
                        case MENU_TIME_AND_DATE_MANUAL_TIME:
                            m_timedate = MENU_TIME_AND_DATE_MANUAL_TIME_HOUR;
                            break;
                        case MENU_TIME_AND_DATE_MANUAL_DATE_YEAR:
                            m_timedate = MENU_TIME_AND_DATE_MANUAL_DATE_YEAR_CHANGE;
                            break;
                        case MENU_TIME_AND_DATE_MANUAL_DATE_MONTH:
                            m_timedate = MENU_TIME_AND_DATE_MANUAL_DATE_MONTH_CHANGE;
                            break;
                        case MENU_TIME_AND_DATE_MANUAL_DATE_DAY:
                            m_timedate = MENU_TIME_AND_DATE_MANUAL_DATE_DAY_CHANGE;
                            break;
                        case MENU_TIME_AND_DATE_MANUAL_TIME_HOUR:
                            m_timedate = MENU_TIME_AND_DATE_MANUAL_TIME_HOUR_CHANGE;
                            break;
                        case MENU_TIME_AND_DATE_MANUAL_TIME_MINUTE:
                            m_timedate = MENU_TIME_AND_DATE_MANUAL_TIME_MINUTE_CHANGE;
                            break;
                        }

                        update_display = true;
                    }
                }
            }
            prev_btn = *btn;
        }

        if (*is_auto_time_date_en != prev_is_auto_time_date_en) {
            prev_is_auto_time_date_en = *is_auto_time_date_en;

            if (!ntp_is_enabled() && *is_auto_time_date_en)
                ntp_init(NTP_SERVER);
            else if (ntp_is_enabled() && *is_auto_time_date_en == false)
                ntp_deinit();

            // save new value to flash
            set_integer_to_storage(STORAGE_KEY_IS_AUTO_TIME_DATE_EN, (int) *is_auto_time_date_en);

            if (m_timedate == MENU_TIME_AND_DATE_AUTO_EN)
                update_display = true;
        }

	if (*timezone_offset_idx!= prev_timezone_offset_idx) {
            prev_timezone_offset_idx= *timezone_offset_idx;

	    // set timezone
	    clock_set_timezone_offset(timezone_offset_list_min[*timezone_offset_idx]);

            // save new value to flash
            set_integer_to_storage(STORAGE_KEY_TIMEZONE_OFFSET_INDEX, *timezone_offset_idx);

            if (m_timedate == MENU_TIME_AND_DATE_TIMEZONE_OFFSET_SET)
                update_display = true;
	}

        // update the display
        if (update_display) {
            printf("m_timedate=%d\r\n", m_timedate);
            update_display = false;

            display_clear_screen();
            // display connection status indication if connected
            if (data->is_connected)
                display_wifi_icon(DISPLAY_COLOR);

            // display date / time screen if enabled
            if (display_date_change_en) {
                display_date_change_en = false;
                clock_get_date(&year, &month, &day);
                display_date(year, month, day, DISPLAY_COLOR);
                vTaskDelay(DISPLAY_DATE_AFTER_CHANGE_MS / portTICK_RATE_MS);
                display_date(year, month, day, !DISPLAY_COLOR);
            } else if (display_time_change_en) {
                display_time_change_en = false;
                clock_get_time(&hour, &minute, NULL);
                display_time(hour, minute, DISPLAY_COLOR);
                vTaskDelay(DISPLAY_TIME_AFTER_CHANGE_MS / portTICK_RATE_MS);
                display_time(hour, minute, !DISPLAY_COLOR);
            }

            switch (m_timedate) {
            case MENU_TIME_AND_DATE_AUTO:
                display_menu("Auto", DISPLAY_COLOR, NULL, !DISPLAY_COLOR);
                break;
            case MENU_TIME_AND_DATE_MANUAL:
                display_menu("Manual", DISPLAY_COLOR, NULL, !DISPLAY_COLOR);
                break;
            case MENU_TIME_AND_DATE_TIMEZONE_OFFSET:
                display_menu("Timezone", DISPLAY_COLOR, "Offset", DISPLAY_COLOR);
                break;
            case MENU_TIME_AND_DATE_AUTO_EN:
                display_menu(*is_auto_time_date_en ? "Enabled" : "Disabled", DISPLAY_COLOR, NULL, !DISPLAY_COLOR);
                break;
            case MENU_TIME_AND_DATE_MANUAL_DATE:
                display_menu("Date", DISPLAY_COLOR, NULL, !DISPLAY_COLOR);
                break;
            case MENU_TIME_AND_DATE_MANUAL_TIME:
                display_menu("Time", DISPLAY_COLOR, NULL, !DISPLAY_COLOR);
                break;
            case MENU_TIME_AND_DATE_TIMEZONE_OFFSET_SET:
                display_timezone(timezone_offset_list_min[*timezone_offset_idx], DISPLAY_COLOR);
                break;
            case MENU_TIME_AND_DATE_MANUAL_DATE_YEAR:
                display_menu("Year", DISPLAY_COLOR, NULL, !DISPLAY_COLOR);
                break;
            case MENU_TIME_AND_DATE_MANUAL_DATE_MONTH:
                display_menu("Month", DISPLAY_COLOR, NULL, !DISPLAY_COLOR);
                break;
            case MENU_TIME_AND_DATE_MANUAL_DATE_DAY:
                display_menu("Day", DISPLAY_COLOR, NULL, !DISPLAY_COLOR);
                break;
            case MENU_TIME_AND_DATE_MANUAL_TIME_HOUR:
                display_menu("Hour", DISPLAY_COLOR, NULL, !DISPLAY_COLOR);
                break;
            case MENU_TIME_AND_DATE_MANUAL_TIME_MINUTE:
                display_menu("Minute", DISPLAY_COLOR, NULL, !DISPLAY_COLOR);
                break;
            case MENU_TIME_AND_DATE_MANUAL_DATE_YEAR_CHANGE:
                clock_get_date(&year, NULL, NULL);
                display_year(year, DISPLAY_COLOR);
                break;
            case MENU_TIME_AND_DATE_MANUAL_DATE_MONTH_CHANGE:
                clock_get_date(NULL, &month, NULL);
                display_month(month, DISPLAY_COLOR);
                break;
            case MENU_TIME_AND_DATE_MANUAL_DATE_DAY_CHANGE:
                clock_get_date(NULL, NULL, &day);
                display_day(day, DISPLAY_COLOR);
                break;
            case MENU_TIME_AND_DATE_MANUAL_TIME_HOUR_CHANGE:
                clock_get_time(&hour, NULL, NULL);
                display_hour(hour, DISPLAY_COLOR);
                break;
            case MENU_TIME_AND_DATE_MANUAL_TIME_MINUTE_CHANGE:
                clock_get_time(NULL, &minute, NULL);
                display_minute(minute, DISPLAY_COLOR);
                break;
            }
        }

        vTaskDelay(1 / portTICK_RATE_MS);
    }

    // exit with display on
    if (screen_off)
       display_on();

    // return new mode
    return next_mode;
}

static app_mode_t menu_communications(app_data_t *data) {
    int *btn = &(data->button_status);
    int prev_btn = *btn;

    // when changing mode, this task should be completed before starting the next mode
    bool exit = false;
    app_mode_t next_mode = data->mode;

    uint8_t m_comms = MENU_COMMUNICATIONS_AP_MODE;
    bool update_display = true;
    time_t btn_power_press_ms = 0;
    time_t btn_forward_press_ms = 0;
    time_t btn_up_press_ms = 0;
    time_t btn_down_press_ms = 0;
    time_t t_last_char_blink_ms = 0;

    char buf[MENU_COMMS_BUF_MAX_LEN + 1];
    size_t input_len = 0;
    char *current_char = NULL;
    bool is_char_change = false;
    uint8_t char_fast_scroll_increment = 0;
    bool last_char_blink = false;

    bool screen_off = false;
    time_t t_screen_on_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;

    while (!exit) {
        if (*btn == prev_btn) {
            if (data->display_settings.is_auto_screen_off_en) {
                if (((xTaskGetTickCount() * portTICK_PERIOD_MS) - t_screen_on_ms) >= (data->display_settings.auto_screen_off_delay_sec * 1000)) {
                    display_off();
                    screen_off = true;
                }
            }

            if (!screen_off) {
                int cur_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
                if (!((*btn >> BUTTON_POWER_BACK_STAT) & 0x01)) { // power button is pressed
                    if ((cur_ms - btn_power_press_ms) >= HEATER_OFF_LONG_PRESS_DUR_MS) {
                        next_mode = APP_MODE_STANDBY;
                        exit = true;
                        btn_power_press_ms = cur_ms;
                    }
                } else if (!((*btn >> BUTTON_UP_STAT) & 0x01)) { // up button is pressed
                    if ((cur_ms - btn_up_press_ms) >= QUICK_SCROLL_LONG_PRESS_DUR_MS) {
                        if ((m_comms == MENU_COMMUNICATIONS_WIFI_AP_SSID_CHANGE) 
                            || (m_comms == MENU_COMMUNICATIONS_WIFI_AP_PASSWORD_CHANGE)) {
                            if (input_len == 0) {
                                current_char = &(buf[0]);
                                *current_char = MENU_COMMS_INPUT_CHARACTER_FIRST_CHAR; // set the first index value to the first char
                                input_len = 1;
                            } else if (*current_char == 0) {
                                *current_char = *(current_char - 1);
                            } else {
                                char_fast_scroll_increment += 2;
                                if (char_fast_scroll_increment > 10) {
                                    char_fast_scroll_increment = 10;
                                }
                                *current_char += char_fast_scroll_increment;
                                if (*current_char > MENU_COMMS_INPUT_CHARACTER_MAX_VAL) {
                                    *current_char = MENU_COMMS_INPUT_CHARACTER_MAX_VAL;
                                }
                            }
                            is_char_change = true;
                        }
                        btn_up_press_ms = cur_ms;
                    }
                } else if (!((*btn >> BUTTON_DOWN_STAT) & 0x01)) { // down button is pressed
                    if ((cur_ms - btn_down_press_ms) >= QUICK_SCROLL_LONG_PRESS_DUR_MS) {
                        if ((m_comms == MENU_COMMUNICATIONS_WIFI_AP_SSID_CHANGE) 
                            || (m_comms == MENU_COMMUNICATIONS_WIFI_AP_PASSWORD_CHANGE)) {
                            if (input_len == 0) {
                                current_char = &(buf[0]);
                                *current_char = MENU_COMMS_INPUT_CHARACTER_FIRST_CHAR; // set the first index value to the first char
                                input_len = 1;
                            } else if (*current_char == 0) {
                                *current_char = *(current_char - 1);
                            } else {
                                char_fast_scroll_increment += 2;
                                if (char_fast_scroll_increment > 10) {
                                    char_fast_scroll_increment = 10;
                                }
                                *current_char -= char_fast_scroll_increment;
                                if (*current_char < MENU_COMMS_INPUT_CHARACTER_MIN_VAL) {
                                    *current_char = MENU_COMMS_INPUT_CHARACTER_MIN_VAL;
                                }
                            }
                            is_char_change = true;
                        }
                        btn_down_press_ms = cur_ms;
                    }
                } else if (!((*btn >> BUTTON_TIMER_FORWARD_STAT) & 0x01)) { // forward button is pressed
                    if ((cur_ms - btn_forward_press_ms) >= HEATER_OFF_LONG_PRESS_DUR_MS) {
                        if (m_comms == MENU_COMMUNICATIONS_WIFI_AP_SSID_CHANGE) {
                            memcpy(data->ap_ssid, buf, strlen(buf));
                            set_string_to_storage(NVS_LUCIDTRON_SSID_KEY, buf);

                            m_comms = MENU_COMMUNICATIONS_WIFI_AP;
                        } else if (m_comms == MENU_COMMUNICATIONS_WIFI_AP_PASSWORD_CHANGE) {
                            memcpy(data->sta_pw, buf, strlen(buf));
                            set_string_to_storage(NVS_LUCIDTRON_PW_KEY, buf);
                            comm_wifi_dev->wifi_client_enable(data->ap_ssid, data->sta_pw);

                            m_comms = MENU_COMMUNICATIONS_WIFI_AP;
                        }
                        btn_forward_press_ms = cur_ms;
                    }
                }
            }
        } else { // any of the buttons was pressed
            if (data->display_settings.is_auto_screen_off_en) {
                t_screen_on_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
            }

            if (screen_off) {
                // enable screen only if all buttons are unpressed
                if (((*btn >> BUTTON_POWER_BACK_STAT) & 0x01) && ((*btn >> BUTTON_UP_STAT) & 0x01)
                    && ((*btn >> BUTTON_DOWN_STAT) & 0x01) && ((*btn >> BUTTON_TIMER_FORWARD_STAT) & 0x01)) {
                    screen_off = false;
                    display_on();
                }
            } else {
                if ((*btn & (1 << BUTTON_POWER_BACK_STAT)) != (prev_btn & (1 << BUTTON_POWER_BACK_STAT))) { // power button toggles
                    if ((*btn >> BUTTON_POWER_BACK_STAT) & 0x01) { // unpressed
                        switch (m_comms) {
                        case MENU_COMMUNICATIONS_AP_MODE:
                        case MENU_COMMUNICATIONS_WIFI_AP:
                        case MENU_COMMUNICATIONS_WPS:
                            exit = true;
                            break;
                        case MENU_COMMUNICATIONS_AP_MODE_EN:
                        case MENU_COMMUNICATIONS_AP_MODE_SSID:
                            m_comms = MENU_COMMUNICATIONS_AP_MODE;
                            break;
                        case MENU_COMMUNICATIONS_WIFI_AP_SSID:
                        case MENU_COMMUNICATIONS_WIFI_AP_PASSWORD:
                            m_comms = MENU_COMMUNICATIONS_WIFI_AP;
                            break;
                        case MENU_COMMUNICATIONS_WPS_EN:
                            m_comms = MENU_COMMUNICATIONS_WPS;
                            break;
                        case MENU_COMMUNICATIONS_AP_MODE_SSID_VAL:
                            m_comms = MENU_COMMUNICATIONS_AP_MODE_SSID;
                            break;
                        case MENU_COMMUNICATIONS_WIFI_AP_SSID_CHANGE:
                            if (input_len > 0) {
                                // delete end character
                                *current_char = 0x00;
                                current_char--;
                                --input_len;
                            } else {
                                m_comms = MENU_COMMUNICATIONS_WIFI_AP_SSID;
                            }
                            break;
                        case MENU_COMMUNICATIONS_WIFI_AP_PASSWORD_CHANGE:
                            if (input_len > 0) {
                                // delete end character
                                *current_char = 0x00;
                                current_char--;
                                --input_len;
                            } else {
                                m_comms = MENU_COMMUNICATIONS_WIFI_AP_PASSWORD;
                            }
                            break;
                        case MENU_COMMUNICATIONS_WPS_EN_INST:
                            // do nothing
                            break;
                        }

                        update_display = true;
                    } else {
                        btn_power_press_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
                    }
                } else if ((*btn & (1 << BUTTON_UP_STAT)) != (prev_btn & (1 << BUTTON_UP_STAT))) { // up button toggles
                    if ((*btn >> BUTTON_UP_STAT) & 0x01) { // unpressed
                        switch (m_comms) {
                        case MENU_COMMUNICATIONS_AP_MODE:
                            m_comms = MENU_COMMUNICATIONS_WIFI_AP;
                            break;
                        case MENU_COMMUNICATIONS_WIFI_AP:
                            m_comms = MENU_COMMUNICATIONS_WPS;
                            break;
                        case MENU_COMMUNICATIONS_WPS:
                            m_comms = MENU_COMMUNICATIONS_AP_MODE;
                            break;
                        case MENU_COMMUNICATIONS_AP_MODE_EN:
                            m_comms = MENU_COMMUNICATIONS_AP_MODE_SSID;
                            break;
                        case MENU_COMMUNICATIONS_AP_MODE_SSID:
                            m_comms = MENU_COMMUNICATIONS_AP_MODE_EN;
                            break;
                        case MENU_COMMUNICATIONS_WIFI_AP_SSID:
                            m_comms = MENU_COMMUNICATIONS_WIFI_AP_PASSWORD;
                            break;
                        case MENU_COMMUNICATIONS_WIFI_AP_PASSWORD:
                            m_comms = MENU_COMMUNICATIONS_WIFI_AP_SSID;
                            break;
                        case MENU_COMMUNICATIONS_WPS_EN:
                            // do nothing
                            break;
                        case MENU_COMMUNICATIONS_AP_MODE_SSID_VAL:
                            // do nothing
                            break;
                        case MENU_COMMUNICATIONS_WPS_EN_INST:
                            // do nothing
                            break;
                        case MENU_COMMUNICATIONS_WIFI_AP_SSID_CHANGE:
                        case MENU_COMMUNICATIONS_WIFI_AP_PASSWORD_CHANGE:
                            // reset multiplier
                            char_fast_scroll_increment = 0;
                            if (input_len == 0) {
                                current_char = &(buf[0]);
                                *current_char = MENU_COMMS_INPUT_CHARACTER_FIRST_CHAR; // set the first index value to the first char
                                input_len = 1;
                            } else if (*current_char == 0) {
                                *current_char = *(current_char - 1);
                            } else if (*current_char == MENU_COMMS_INPUT_CHARACTER_MAX_VAL) {
                                *current_char = MENU_COMMS_INPUT_CHARACTER_MIN_VAL;
                            } else {
                                *current_char += 1;
                            }
                            is_char_change = true;
                            break;
                        }

                        update_display = true;
                    } else {
                        btn_up_press_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
                    }
                } else if ((*btn & (1 << BUTTON_DOWN_STAT)) != (prev_btn & (1 << BUTTON_DOWN_STAT))) { // down button toggles
                    if ((*btn >> BUTTON_DOWN_STAT) & 0x01) { // unpressed
                        switch (m_comms) {
                        case MENU_COMMUNICATIONS_AP_MODE:
                            m_comms = MENU_COMMUNICATIONS_WPS;
                            break;
                        case MENU_COMMUNICATIONS_WIFI_AP:
                            m_comms = MENU_COMMUNICATIONS_AP_MODE;
                            break;
                        case MENU_COMMUNICATIONS_WPS:
                            m_comms = MENU_COMMUNICATIONS_WIFI_AP;
                            break;
                        case MENU_COMMUNICATIONS_AP_MODE_EN:
                            m_comms = MENU_COMMUNICATIONS_AP_MODE_SSID;
                            break;
                        case MENU_COMMUNICATIONS_AP_MODE_SSID:
                            m_comms = MENU_COMMUNICATIONS_AP_MODE_EN;
                            break;
                        case MENU_COMMUNICATIONS_WIFI_AP_SSID:
                            m_comms = MENU_COMMUNICATIONS_WIFI_AP_PASSWORD;
                            break;
                        case MENU_COMMUNICATIONS_WIFI_AP_PASSWORD:
                            m_comms = MENU_COMMUNICATIONS_WIFI_AP_SSID;
                            break;
                        case MENU_COMMUNICATIONS_WPS_EN:
                            // do nothing
                            break;
                        case MENU_COMMUNICATIONS_AP_MODE_SSID_VAL:
                            // do nothing
                            break;
                        case MENU_COMMUNICATIONS_WPS_EN_INST:
                            // disable WPS
                            comm_wifi_dev->wps_disable();
                            m_comms = MENU_COMMUNICATIONS_WPS;
                            break;
                        case MENU_COMMUNICATIONS_WIFI_AP_SSID_CHANGE:
                        case MENU_COMMUNICATIONS_WIFI_AP_PASSWORD_CHANGE:
                            // input
                            // reset multiplier
                            char_fast_scroll_increment = 0;
                            if (input_len == 0) {
                                current_char = &(buf[0]);
                                *current_char = MENU_COMMS_INPUT_CHARACTER_FIRST_CHAR; // set the first index value to the first char
                                input_len = 1;
                            } else if (*current_char == 0) {
                                *current_char = *(current_char - 1);
                            } else if (*current_char == MENU_COMMS_INPUT_CHARACTER_MIN_VAL) {
                                *current_char = MENU_COMMS_INPUT_CHARACTER_MAX_VAL;
                            } else {
                                *current_char -= 1;
                            }
                            is_char_change = true;
                            break;
                        }
                        update_display = true;
                    } else {
                        btn_down_press_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
                    }
                } else if ((*btn & (1 << BUTTON_TIMER_FORWARD_STAT)) != (prev_btn & (1 << BUTTON_TIMER_FORWARD_STAT))) { // timer button toggles
                    if ((*btn >> BUTTON_TIMER_FORWARD_STAT) & 0x01) { // unpressed
                        switch (m_comms) {
                        case MENU_COMMUNICATIONS_AP_MODE:
                            m_comms = MENU_COMMUNICATIONS_AP_MODE_EN;
                            break;
                        case MENU_COMMUNICATIONS_WIFI_AP:
                            // get Wi-Fi AP info
                            esp_wifi_sta_get_ap_info(&ap_info);
                            memset(data->sta_ssid, 0, sizeof(data->sta_ssid));
                            memcpy(data->sta_ssid, ap_info.ssid, strlen((char *) ap_info.ssid));
                            m_comms = MENU_COMMUNICATIONS_WIFI_AP_SSID;
                            break;
                        case MENU_COMMUNICATIONS_WPS:
                            m_comms = MENU_COMMUNICATIONS_WPS_EN;
                            break;
                        case MENU_COMMUNICATIONS_AP_MODE_EN:
                            // enable AP mode
                            if (comm_wifi_dev->is_wifi_ap_enabled())
                                comm_wifi_dev->wifi_ap_disable();
                            else
                                comm_wifi_dev->wifi_ap_enable(comm_wifi_dev->wifi_ap_ssid, comm_wifi_dev->wifi_ap_pw);
                            break;
                        case MENU_COMMUNICATIONS_AP_MODE_SSID:
                            m_comms = MENU_COMMUNICATIONS_AP_MODE_SSID_VAL;
                            break;
                        case MENU_COMMUNICATIONS_WIFI_AP_SSID:
                            memset(buf, 0, MENU_COMMS_BUF_MAX_LEN + 1);
                            get_string_from_storage(NVS_LUCIDTRON_SSID_KEY, buf);
                            input_len = strlen(buf);

                            // set current char
                            current_char = &(buf[input_len]);

                            is_char_change = false;

                            m_comms = MENU_COMMUNICATIONS_WIFI_AP_SSID_CHANGE;
                            t_last_char_blink_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
                            last_char_blink = false;
                            break;
                        case MENU_COMMUNICATIONS_WIFI_AP_PASSWORD:
                            memset(buf, 0, MENU_COMMS_BUF_MAX_LEN + 1);
                            get_string_from_storage(NVS_LUCIDTRON_PW_KEY, buf);
                            input_len = strlen(buf);

                            // set current char
                            current_char = &(buf[input_len]);

                            is_char_change = false;

                            m_comms = MENU_COMMUNICATIONS_WIFI_AP_PASSWORD_CHANGE;
                            t_last_char_blink_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
                            last_char_blink = false;
                            break;
                        case MENU_COMMUNICATIONS_WPS_EN:
                            // WPS
                            // start WPS
                            comm_wifi_dev->wps_enable();
                            m_comms = MENU_COMMUNICATIONS_WPS_EN_INST;
                            break;
                        case MENU_COMMUNICATIONS_AP_MODE_SSID_VAL:
                            // do nothing
                            break;
                        case MENU_COMMUNICATIONS_WIFI_AP_SSID_CHANGE:
                        case MENU_COMMUNICATIONS_WIFI_AP_PASSWORD_CHANGE:
                            current_char++;
                            *current_char = 0;
                            ++input_len;
                            is_char_change = true;
                            break;
                        case MENU_COMMUNICATIONS_WPS_EN_INST:
                            // do nothing
                            break;
                        }

                        update_display = true;
                    } else {
                        btn_forward_press_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
                    }
                }
            }
            prev_btn = *btn;
        }

        if (is_char_change) {
            is_char_change = false;
            update_display = true;
        }

        if ((m_comms == MENU_COMMUNICATIONS_WIFI_AP_SSID_CHANGE)
            || (m_comms == MENU_COMMUNICATIONS_WIFI_AP_PASSWORD_CHANGE)) {
            int cur_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
            if ((cur_ms - t_last_char_blink_ms) >= 1000) {
                last_char_blink = !last_char_blink;
                t_last_char_blink_ms = cur_ms;
                if (*current_char != 0) {
                    update_display = true;
                }
            }
        }

        // update the display
        if (update_display) {
            printf("m_comms=%d\r\n", m_comms);

            display_clear_screen();
            // display connection status indication if connected
            if (data->is_connected)
                display_wifi_icon(DISPLAY_COLOR);

            switch (m_comms) {
            case MENU_COMMUNICATIONS_AP_MODE:
                printf("MENU_COMMUNICATIONS_AP_MODE\r\n");
                display_menu("AP Mode", DISPLAY_COLOR, NULL, !DISPLAY_COLOR);
                break;
            case MENU_COMMUNICATIONS_WIFI_AP:
                display_menu("Wi-Fi", DISPLAY_COLOR, "connection", DISPLAY_COLOR);
                printf("MENU_COMMUNICATIONS_WIFI_AP\r\n");
                break;
            case MENU_COMMUNICATIONS_WPS:
                printf("MENU_COMMUNICATIONS_WPS\r\n");
                display_menu("WPS", DISPLAY_COLOR, NULL, !DISPLAY_COLOR);
                break;
            case MENU_COMMUNICATIONS_AP_MODE_EN:
                printf("MENU_COMMUNICATIONS_AP_MODE_EN\r\n");
                display_menu(comm_wifi_dev->is_wifi_ap_enabled() ? "Disable" : "Enable", DISPLAY_COLOR, "AP mode", DISPLAY_COLOR);
                break;
            case MENU_COMMUNICATIONS_AP_MODE_SSID:
                printf("MENU_COMMUNICATIONS_AP_MODE_SSID\r\n");
                display_menu("SSID", DISPLAY_COLOR, NULL, !DISPLAY_COLOR);
                break;
            case MENU_COMMUNICATIONS_WIFI_AP_SSID:
                printf("MENU_COMMUNICATIONS_WIFI_AP_SSID\r\n");
                display_menu("Network", DISPLAY_COLOR, "Name", DISPLAY_COLOR);
                break;
            case MENU_COMMUNICATIONS_WIFI_AP_PASSWORD:
                printf("MENU_COMMUNICATIONS_WIFI_AP_PASSWORD\r\n");
                display_menu("Password", DISPLAY_COLOR, NULL, !DISPLAY_COLOR);
                break;
            case MENU_COMMUNICATIONS_WPS_EN:
                printf("MENU_COMMUNICATIONS_WPS_EN\r\n");
                display_menu("Enable", DISPLAY_COLOR, "WPS Mode", DISPLAY_COLOR);
                break;
            case MENU_COMMUNICATIONS_AP_MODE_SSID_VAL:
                printf("MENU_COMMUNICATIONS_AP_MODE_SSID_VAL\r\n");
                display_ssid(comm_wifi_dev->wifi_ap_ssid, DISPLAY_COLOR);
                break;
            case MENU_COMMUNICATIONS_WIFI_AP_SSID_CHANGE:
                printf("MENU_COMMUNICATIONS_WIFI_AP_SSID_CHANGE\r\n");
                display_ssid(buf, DISPLAY_COLOR);
                if (last_char_blink) {
                    display_clear_last_char_ssid_pw(*current_char);
                    printf("\r\nlast char %c\r\n", *current_char);
                }
                break;
            case MENU_COMMUNICATIONS_WIFI_AP_PASSWORD_CHANGE:
                printf("MENU_COMMUNICATIONS_WIFI_AP_PASSWORD_CHANGE\r\n");
                display_password(buf, DISPLAY_COLOR);
                if (last_char_blink) {
                    display_clear_last_char_ssid_pw(*current_char);
                    printf("\r\nlast char %c\r\n", *current_char);
                }
                break;
            case MENU_COMMUNICATIONS_WPS_EN_INST:
                printf("MENU_COMMUNICATIONS_WPS_EN_INST\r\n");
                display_wps_mode_msg(DISPLAY_COLOR);
                break;
            }

            update_display = false;
        }

        vTaskDelay(1 / portTICK_RATE_MS);
    }

    // exit with display on
    if (screen_off)
       display_on();

    // return new mode
    return next_mode;
}

static app_mode_t menu_settings(app_data_t *data) {
    int *btn = &(data->button_status);
    int prev_btn = *btn;

    // when changing mode, this task should be completed before starting the next mode
    bool exit = false;
    app_mode_t next_mode = data->mode;

    uint8_t m_settings = MENU_SETTINGS_TEMPERATURE_UNIT;
    bool update_display = true;
    time_t btn_power_press_ms = 0;

    bool is_settings_changed = false;
    int *temp_hysteresis_c = &(data->settings.temperature_hysteresis_celsius);
    int *temp_hysteresis_f = &(data->settings.temperature_hysteresis_fahrenheit);

    bool screen_off = false;
    time_t t_screen_on_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;

    while (!exit) {
        if (*btn == prev_btn) {
            if (data->display_settings.is_auto_screen_off_en) {
                if (((xTaskGetTickCount() * portTICK_PERIOD_MS) - t_screen_on_ms) >= (data->display_settings.auto_screen_off_delay_sec * 1000)) {
                    display_off();
                    screen_off = true;
                }
            }

            if (!screen_off) {
                if (!((*btn >> BUTTON_POWER_BACK_STAT) & 0x01)) { // power button is pressed
                    int cur_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
                    if ((cur_ms - btn_power_press_ms) >= HEATER_OFF_LONG_PRESS_DUR_MS) {
                        next_mode = APP_MODE_STANDBY;
                        exit = true;
                        btn_power_press_ms = cur_ms;
                    }
                }
            }
        } else { // any of the buttons was pressed
            if (data->display_settings.is_auto_screen_off_en) {
                t_screen_on_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
            }

            if (screen_off) {
                // enable screen only if all buttons are unpressed
                if (((*btn >> BUTTON_POWER_BACK_STAT) & 0x01) && ((*btn >> BUTTON_UP_STAT) & 0x01)
                    && ((*btn >> BUTTON_DOWN_STAT) & 0x01) && ((*btn >> BUTTON_TIMER_FORWARD_STAT) & 0x01)) {
                    screen_off = false;
                    display_on();
                }
            } else {
                if ((*btn & (1 << BUTTON_POWER_BACK_STAT)) != (prev_btn & (1 << BUTTON_POWER_BACK_STAT))) { // power button toggles
                    if ((*btn >> BUTTON_POWER_BACK_STAT) & 0x01) { // unpressed
                        switch (m_settings) {
                        case MENU_SETTINGS_TEMPERATURE_UNIT:
                        case MENU_SETTINGS_CHILD_LOCK:
                        case MENU_SETTINGS_PILOT_LIGHT:
                        case MENU_SETTINGS_NIGHT_LIGHT:
                        case MENU_SETTINGS_TEMPERATURE_HYSTERESIS:
                            exit = true;
                            break;
                        case MENU_SETTINGS_TEMPERATURE_UNIT_CHANGE:
                            m_settings = MENU_SETTINGS_TEMPERATURE_UNIT;
                            break;
                        case MENU_SETTINGS_CHILD_LOCK_EN:
                            m_settings = MENU_SETTINGS_CHILD_LOCK;
                            break;
                        case MENU_SETTINGS_PILOT_LIGHT_EN:
                            m_settings = MENU_SETTINGS_PILOT_LIGHT;
                            break;
                        case MENU_SETTINGS_NIGHT_LIGHT_CFG:
                            m_settings = MENU_SETTINGS_NIGHT_LIGHT;
                            break;
                        case MENU_SETTINGS_TEMPERATURE_HYSTERESIS_CHANGE_WARNING:
                        case MENU_SETTINGS_TEMPERATURE_HYSTERESIS_CHANGE:
                            m_settings = MENU_SETTINGS_TEMPERATURE_HYSTERESIS;
                            break;
                        }

                        update_display = true;
                    } else {
                        btn_power_press_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
                    }
                } else if ((*btn & (1 << BUTTON_UP_STAT)) != (prev_btn & (1 << BUTTON_UP_STAT))) { // up button toggles
                    if ((*btn >> BUTTON_UP_STAT) & 0x01) { // unpressed
                        switch (m_settings) {
                        case MENU_SETTINGS_TEMPERATURE_UNIT:
                            m_settings = MENU_SETTINGS_CHILD_LOCK;
                            break;
                        case MENU_SETTINGS_CHILD_LOCK:
                            m_settings = MENU_SETTINGS_PILOT_LIGHT;
                            break;
                        case MENU_SETTINGS_PILOT_LIGHT:
                            m_settings = MENU_SETTINGS_NIGHT_LIGHT;
                            break;
                        case MENU_SETTINGS_NIGHT_LIGHT:
                            m_settings = MENU_SETTINGS_TEMPERATURE_HYSTERESIS;
                            break;
                        case MENU_SETTINGS_TEMPERATURE_HYSTERESIS:
                            m_settings = MENU_SETTINGS_TEMPERATURE_UNIT;
                            break;
                        case MENU_SETTINGS_TEMPERATURE_UNIT_CHANGE:
                            is_settings_changed = true;
                            // Fahrenheit <--> Celsius
                            if (data->settings.temperature_unit == TEMP_UNIT_CELSIUS)
                                data->settings.temperature_unit = TEMP_UNIT_FAHRENHEIT;
                            else
                                data->settings.temperature_unit =  TEMP_UNIT_CELSIUS;
                            break;
                        case MENU_SETTINGS_CHILD_LOCK_EN:
                            is_settings_changed = true;
                            // Enable <--> Disable
                            data->settings.is_child_lock_en = !data->settings.is_child_lock_en;
                            break;
                        case MENU_SETTINGS_PILOT_LIGHT_EN:
                            is_settings_changed = true;
                            // Enable <--> Disable
                            data->settings.is_dim_pilot_light_en = !data->settings.is_dim_pilot_light_en;
                            break;
                        case MENU_SETTINGS_NIGHT_LIGHT_CFG:
                            is_settings_changed = true;
                            // Auto <--> Off
                            data->settings.is_night_light_auto_brightness_en = !data->settings.is_night_light_auto_brightness_en;
                            break;
                        case MENU_SETTINGS_TEMPERATURE_HYSTERESIS_CHANGE:
                            if (data->settings.temperature_unit == TEMP_UNIT_CELSIUS) {
                                if (*temp_hysteresis_c < TEMPERATURE_HYSTERESIS_CELSIUS_MAX) {
                                    is_settings_changed = true;
                                    *temp_hysteresis_c += 1;
                                    // set the other unit
//                                    *temp_hysteresis_f = celsius_to_fahr(*temp_hysteresis_c);
                                }
                            } else {
                                if (*temp_hysteresis_f < TEMPERATURE_HYSTERESIS_FAHRENHEIT_MAX) {
                                    is_settings_changed = true;
                                    *temp_hysteresis_f += 1;
                                    // set the other unit
//                                    *temp_hysteresis_c = fahr_to_celsius(*temp_hysteresis_f);
                                }
                            }
                            break;
                        }

                        update_display = true;
                    }
                } else if ((*btn & (1 << BUTTON_DOWN_STAT)) != (prev_btn & (1 << BUTTON_DOWN_STAT))) { // down button toggles
                    if ((*btn >> BUTTON_DOWN_STAT) & 0x01) { // unpressed
                        switch (m_settings) {
                        case MENU_SETTINGS_TEMPERATURE_UNIT:
                            m_settings = MENU_SETTINGS_TEMPERATURE_HYSTERESIS;
                            break;
                        case MENU_SETTINGS_CHILD_LOCK:
                            m_settings = MENU_SETTINGS_TEMPERATURE_UNIT;
                            break;
                        case MENU_SETTINGS_PILOT_LIGHT:
                            m_settings = MENU_SETTINGS_CHILD_LOCK;
                            break;
                        case MENU_SETTINGS_NIGHT_LIGHT:
                            m_settings = MENU_SETTINGS_PILOT_LIGHT;
                            break;
                        case MENU_SETTINGS_TEMPERATURE_HYSTERESIS:
                            m_settings = MENU_SETTINGS_NIGHT_LIGHT;
                            break;
                        case MENU_SETTINGS_TEMPERATURE_UNIT_CHANGE:
                            is_settings_changed = true;
                            // Fahrenheit <--> Celsius
                            if (data->settings.temperature_unit == TEMP_UNIT_CELSIUS)
                                data->settings.temperature_unit = TEMP_UNIT_FAHRENHEIT;
                            else
                                data->settings.temperature_unit =  TEMP_UNIT_CELSIUS;
                            break;
                        case MENU_SETTINGS_CHILD_LOCK_EN:
                            is_settings_changed = true;
                            // Enable <--> Disable
                            data->settings.is_child_lock_en = !data->settings.is_child_lock_en;
                            break;
                        case MENU_SETTINGS_PILOT_LIGHT_EN:
                            is_settings_changed = true;
                            // Enable <--> Disable
                            data->settings.is_dim_pilot_light_en = !data->settings.is_dim_pilot_light_en;
                            break;
                        case MENU_SETTINGS_NIGHT_LIGHT_CFG:
                            is_settings_changed = true;
                            // Auto <--> Off
                            data->settings.is_night_light_auto_brightness_en = !data->settings.is_night_light_auto_brightness_en;
                            break;
                        case MENU_SETTINGS_TEMPERATURE_HYSTERESIS_CHANGE:
                            if (data->settings.temperature_unit == TEMP_UNIT_CELSIUS) {
                                if (*temp_hysteresis_c > TEMPERATURE_HYSTERESIS_CELSIUS_MIN) {
                                    is_settings_changed = true;
                                    *temp_hysteresis_c -= 1;
                                    // set the other unit
//                                    *temp_hysteresis_f = celsius_to_fahr(*temp_hysteresis_c);
                                }
                            } else {
                                if (*temp_hysteresis_f > TEMPERATURE_HYSTERESIS_FAHRENHEIT_MIN) {
                                    is_settings_changed = true;
                                    *temp_hysteresis_f -= 1;
                                    // set the other unit
//                                    *temp_hysteresis_c = fahr_to_celsius(*temp_hysteresis_f);
                                }
                            }
                            break;
                        }

                        update_display = true;
                    }
                } else if ((*btn & (1 << BUTTON_TIMER_FORWARD_STAT)) != (prev_btn & (1 << BUTTON_TIMER_FORWARD_STAT))) { // timer button toggles
                    if ((*btn >> BUTTON_TIMER_FORWARD_STAT) & 0x01) { // unpressed
                        switch (m_settings) {
                        case MENU_SETTINGS_TEMPERATURE_UNIT:
                            m_settings = MENU_SETTINGS_TEMPERATURE_UNIT_CHANGE;
                            break;
                        case MENU_SETTINGS_CHILD_LOCK:
                            m_settings = MENU_SETTINGS_CHILD_LOCK_EN;
                            break;
                        case MENU_SETTINGS_PILOT_LIGHT:
                            m_settings = MENU_SETTINGS_PILOT_LIGHT_EN;
                            break;
                        case MENU_SETTINGS_NIGHT_LIGHT:
                            m_settings = MENU_SETTINGS_NIGHT_LIGHT_CFG;
                            break;
                        case MENU_SETTINGS_TEMPERATURE_HYSTERESIS:
                            m_settings = MENU_SETTINGS_TEMPERATURE_HYSTERESIS_CHANGE_WARNING;
                            break;
                        case MENU_SETTINGS_TEMPERATURE_HYSTERESIS_CHANGE_WARNING:
                            m_settings = MENU_SETTINGS_TEMPERATURE_HYSTERESIS_CHANGE;
                            break;
                        }

                        update_display = true;
                    }
                }
            }
            prev_btn = *btn;
        }

        // update the display
        if (update_display) {
            printf("m_settings=%d\r\n", m_settings);

            display_clear_screen();
            // display connection status indication if connected
            if (data->is_connected)
                display_wifi_icon(DISPLAY_COLOR);

            switch (m_settings) {
            case MENU_SETTINGS_TEMPERATURE_UNIT:
                printf("MENU_SETTINGS_TEMPERATURE_UNIT\r\n");
                display_menu("Temp Unit", DISPLAY_COLOR, NULL, !DISPLAY_COLOR);
                break;
            case MENU_SETTINGS_CHILD_LOCK:
                printf("MENU_SETTINGS_CHILD_LOCK\r\n");
                display_menu("Child Lock", DISPLAY_COLOR, NULL, !DISPLAY_COLOR);
                break;
            case MENU_SETTINGS_PILOT_LIGHT:
                printf("MENU_SETTINGS_PILOT_LIGHT\r\n");
                display_menu("Auto Dim", DISPLAY_COLOR, "Pilot Light", DISPLAY_COLOR);
                break;
            case MENU_SETTINGS_TEMPERATURE_HYSTERESIS:
                printf("MENU_SETTINGS_TEMPERATURE_HYSTERESIS\r\n");
                display_menu("Hysteresis", DISPLAY_COLOR, NULL, !DISPLAY_COLOR);
                break;
            case MENU_SETTINGS_NIGHT_LIGHT:
                printf("MENU_SETTINGS_NIGHT_LIGHT\r\n");
                display_menu("Night", DISPLAY_COLOR, "Light", DISPLAY_COLOR);
                break;
            case MENU_SETTINGS_TEMPERATURE_UNIT_CHANGE:
                printf("MENU_SETTINGS_TEMPERATURE_UNIT_CHANGE\r\n");
                display_menu(data->settings.temperature_unit == TEMP_UNIT_CELSIUS ? "Metric(°C)" : "US(°F)", DISPLAY_COLOR, NULL, !DISPLAY_COLOR);
                break;
            case MENU_SETTINGS_CHILD_LOCK_EN:
                printf("MENU_SETTINGS_CHILD_LOCK_EN\r\n");
                display_menu(data->settings.is_child_lock_en ? "Enabled" : "Disabled", DISPLAY_COLOR, NULL, !DISPLAY_COLOR);
                break;
            case MENU_SETTINGS_PILOT_LIGHT_EN:
                printf("MENU_SETTINGS_PILOT_LIGHT_EN\r\n");
                display_menu(data->settings.is_dim_pilot_light_en ? "Enabled" : "Disabled", DISPLAY_COLOR, NULL, !DISPLAY_COLOR);
                break;
            case MENU_SETTINGS_TEMPERATURE_HYSTERESIS_CHANGE_WARNING:
                printf("MENU_SETTINGS_TEMPERATURE_HYSTERESIS_CHANGE_WARNING\r\n");
                display_hysteresis_setting_warning(DISPLAY_COLOR);
                break;
            case MENU_SETTINGS_TEMPERATURE_HYSTERESIS_CHANGE:
                printf("MENU_SETTINGS_TEMPERATURE_HYSTERESIS_CHANGE\r\n");
                display_temperature(data->settings.temperature_unit == TEMP_UNIT_CELSIUS ? *temp_hysteresis_c : *temp_hysteresis_f, DISPLAY_COLOR);
                break;
            case MENU_SETTINGS_NIGHT_LIGHT_CFG:
                printf("MENU_SETTINGS_NIGHT_LIGHT_CFG\r\n");
                display_menu((data->settings.is_night_light_auto_brightness_en) ? "Auto" : "Off", DISPLAY_COLOR, NULL, !DISPLAY_COLOR);
                break;
            }

            update_display = false;
        }

        vTaskDelay(1 / portTICK_RATE_MS);
    }

    // exit with display on
    if (screen_off)
       display_on();

    // save settings if changed
    if (is_settings_changed)
        set_data_to_storage(STORAGE_KEY_SETTINGS, (void *) &data->settings, sizeof(settings_t));

    // return new mode
    return next_mode;
}

static app_mode_t menu_display_settings(app_data_t *data) {
    int *btn = &(data->button_status);
    int prev_btn = *btn;

    // when changing mode, this task should be completed before starting the next mode
    bool exit = false;
    app_mode_t next_mode = data->mode;

    uint8_t m_display_settings = MENU_DISPLAY_SETTINGS_BRIGHTNESS;
    bool update_display = true;
    time_t btn_power_press_ms = 0;

    bool is_display_settings_changed = false;

    bool screen_off = false;
    time_t t_screen_on_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;

    while (!exit) {
        if (*btn == prev_btn) {
            if (data->display_settings.is_auto_screen_off_en) {
                if (((xTaskGetTickCount() * portTICK_PERIOD_MS) - t_screen_on_ms) >= (data->display_settings.auto_screen_off_delay_sec * 1000)) {
                    display_off();
                    screen_off = true;
                }
            }

            if (!screen_off) {
                if (!((*btn >> BUTTON_POWER_BACK_STAT) & 0x01)) { // power button is pressed
                    int cur_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
                    if ((cur_ms - btn_power_press_ms) >= HEATER_OFF_LONG_PRESS_DUR_MS) {
                        next_mode = APP_MODE_STANDBY;
                        exit = true;
                        btn_power_press_ms = cur_ms;
                    }
                }
            }
        } else {
            if (data->display_settings.is_auto_screen_off_en) {
                t_screen_on_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
            }

            if (screen_off) {
                // enable screen only if all buttons are unpressed
                if (((*btn >> BUTTON_POWER_BACK_STAT) & 0x01) && ((*btn >> BUTTON_UP_STAT) & 0x01)
                    && ((*btn >> BUTTON_DOWN_STAT) & 0x01) && ((*btn >> BUTTON_TIMER_FORWARD_STAT) & 0x01)) {
                    screen_off = false;
                    display_on();
                }
            } else {
                if ((*btn & (1 << BUTTON_POWER_BACK_STAT)) != (prev_btn & (1 << BUTTON_POWER_BACK_STAT))) { // power button toggles
                    if ((*btn >> BUTTON_POWER_BACK_STAT) & 0x01) { // unpressed
                        switch (m_display_settings) {
                        case MENU_DISPLAY_SETTINGS_BRIGHTNESS:
                        case MENU_DISPLAY_SETTINGS_AUTO_SCREEN_OFF:
                        case MENU_DISPLAY_SETTINGS_AUTO_SCREEN_OFF_DELAY:
                            exit = true;
                            break;
                        case MENU_DISPLAY_SETTINGS_BRIGHTNESS_MANUAL:
                        case MENU_DISPLAY_SETTINGS_BRIGHTNESS_AUTO:
                            m_display_settings = MENU_DISPLAY_SETTINGS_BRIGHTNESS;
                            break;
                        case MENU_DISPLAY_SETTINGS_AUTO_SCREEN_OFF_EN:
                            m_display_settings = MENU_DISPLAY_SETTINGS_AUTO_SCREEN_OFF;
                            break;
                        case MENU_DISPLAY_SETTINGS_AUTO_SCREEN_OFF_DELAY_CHANGE:
                            m_display_settings = MENU_DISPLAY_SETTINGS_AUTO_SCREEN_OFF_DELAY;
                            break;
                        case MENU_DISPLAY_SETTINGS_BRIGHTNESS_MANUAL_CHANGE:
                            m_display_settings = MENU_DISPLAY_SETTINGS_BRIGHTNESS_MANUAL;
                            break;
                        case MENU_DISPLAY_SETTINGS_BRIGHTNESS_AUTO_EN:
                            m_display_settings = MENU_DISPLAY_SETTINGS_BRIGHTNESS_AUTO;
                            break;
                        }

                        update_display = true;
                    } else {
                        btn_power_press_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
                    }
                } else if ((*btn & (1 << BUTTON_UP_STAT)) != (prev_btn & (1 << BUTTON_UP_STAT))) { // up button toggles
                    if ((*btn >> BUTTON_UP_STAT) & 0x01) { // unpressed
                        switch (m_display_settings) {
                        case MENU_DISPLAY_SETTINGS_BRIGHTNESS:
                            m_display_settings = MENU_DISPLAY_SETTINGS_AUTO_SCREEN_OFF;
                            break;
                        case MENU_DISPLAY_SETTINGS_AUTO_SCREEN_OFF:
                            m_display_settings = MENU_DISPLAY_SETTINGS_AUTO_SCREEN_OFF_DELAY;
                            break;
                        case MENU_DISPLAY_SETTINGS_AUTO_SCREEN_OFF_DELAY:
                            m_display_settings = MENU_DISPLAY_SETTINGS_BRIGHTNESS;
                            break;
                        case MENU_DISPLAY_SETTINGS_BRIGHTNESS_MANUAL:
                            m_display_settings = MENU_DISPLAY_SETTINGS_BRIGHTNESS_AUTO;
                            break;
                        case MENU_DISPLAY_SETTINGS_BRIGHTNESS_AUTO:
                            m_display_settings = MENU_DISPLAY_SETTINGS_BRIGHTNESS_MANUAL;
                            break;
                        case MENU_DISPLAY_SETTINGS_AUTO_SCREEN_OFF_EN:
                            is_display_settings_changed = true;
                            data->display_settings.is_auto_screen_off_en = !data->display_settings.is_auto_screen_off_en;
                            t_screen_on_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
                            break;
                        case MENU_DISPLAY_SETTINGS_AUTO_SCREEN_OFF_DELAY_CHANGE:
                            is_display_settings_changed = true;
                            if (data->display_settings.auto_screen_off_delay_sec < AUTO_SCREEN_OFF_DELAY_SEC_MAX)
                                data->display_settings.auto_screen_off_delay_sec += AUTO_SCREEN_OFF_DELAY_SEC_INCREMENT;
                            break;
                        case MENU_DISPLAY_SETTINGS_BRIGHTNESS_MANUAL_CHANGE:
                            is_display_settings_changed = true;
                            if (data->display_settings.display_brightness < DISPLAY_BRIGHTNESS_MAX) {
                                data->display_settings.display_brightness += DISPLAY_BRIGHTNESS_INCREMENT;
                                if (data->display_settings.display_brightness > DISPLAY_BRIGHTNESS_MAX) {
                                    data->display_settings.display_brightness = DISPLAY_BRIGHTNESS_MAX;
                                }
 
                            }
                            break;
                        case MENU_DISPLAY_SETTINGS_BRIGHTNESS_AUTO_EN:
                            is_display_settings_changed = true;
                            data->display_settings.is_auto_display_brightness_en = !data->display_settings.is_auto_display_brightness_en;
                            if (data->display_settings.is_auto_display_brightness_en == false) {
                                data->display_settings.display_brightness = DISPLAY_BRIGHTNESS_DEF;
                            }
                            break;
                        }

                        update_display = true;
                    }
                } else if ((*btn & (1 << BUTTON_DOWN_STAT)) != (prev_btn & (1 << BUTTON_DOWN_STAT))) { // down button toggles
                    if ((*btn >> BUTTON_DOWN_STAT) & 0x01) { // unpressed
                        switch (m_display_settings) {
                        case MENU_DISPLAY_SETTINGS_BRIGHTNESS:
                            m_display_settings = MENU_DISPLAY_SETTINGS_AUTO_SCREEN_OFF_DELAY;
                            break;
                        case MENU_DISPLAY_SETTINGS_AUTO_SCREEN_OFF:
                            m_display_settings = MENU_DISPLAY_SETTINGS_BRIGHTNESS;
                            break;
                        case MENU_DISPLAY_SETTINGS_AUTO_SCREEN_OFF_DELAY:
                            m_display_settings = MENU_DISPLAY_SETTINGS_AUTO_SCREEN_OFF;
                            break;
                        case MENU_DISPLAY_SETTINGS_BRIGHTNESS_MANUAL:
                            m_display_settings = MENU_DISPLAY_SETTINGS_BRIGHTNESS_AUTO;
                            break;
                        case MENU_DISPLAY_SETTINGS_BRIGHTNESS_AUTO:
                            m_display_settings = MENU_DISPLAY_SETTINGS_BRIGHTNESS_MANUAL;
                            break;
                        case MENU_DISPLAY_SETTINGS_AUTO_SCREEN_OFF_EN:
                            is_display_settings_changed = true;
                            data->display_settings.is_auto_screen_off_en = !data->display_settings.is_auto_screen_off_en;
                            t_screen_on_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
                            break;
                        case MENU_DISPLAY_SETTINGS_AUTO_SCREEN_OFF_DELAY_CHANGE:
                            is_display_settings_changed = true;
                            if (data->display_settings.auto_screen_off_delay_sec > AUTO_SCREEN_OFF_DELAY_SEC_MIN)
                                data->display_settings.auto_screen_off_delay_sec -= AUTO_SCREEN_OFF_DELAY_SEC_INCREMENT;
                            break;
                       case MENU_DISPLAY_SETTINGS_BRIGHTNESS_MANUAL_CHANGE:
                            is_display_settings_changed = true;
                            if (data->display_settings.display_brightness > DISPLAY_BRIGHTNESS_MIN) {
                                data->display_settings.display_brightness -= DISPLAY_BRIGHTNESS_INCREMENT;
                                if (data->display_settings.display_brightness < DISPLAY_BRIGHTNESS_MIN) {
                                    data->display_settings.display_brightness = DISPLAY_BRIGHTNESS_MIN;
                                }
                            }
                            break;
                        case MENU_DISPLAY_SETTINGS_BRIGHTNESS_AUTO_EN:
                            is_display_settings_changed = true;
                            data->display_settings.is_auto_display_brightness_en = !data->display_settings.is_auto_display_brightness_en;
                            if (data->display_settings.is_auto_display_brightness_en == false) {
                                data->display_settings.display_brightness = DISPLAY_BRIGHTNESS_DEF;
                            }
                            break;
                        }

                        update_display = true;
                    }
                } else if ((*btn & (1 << BUTTON_TIMER_FORWARD_STAT)) != (prev_btn & (1 << BUTTON_TIMER_FORWARD_STAT))) { // timer button toggles
                    if ((*btn >> BUTTON_TIMER_FORWARD_STAT) & 0x01) { // unpressed
                        switch (m_display_settings) {
                        case MENU_DISPLAY_SETTINGS_BRIGHTNESS:
                            m_display_settings = MENU_DISPLAY_SETTINGS_BRIGHTNESS_MANUAL;
                            break;
                        case MENU_DISPLAY_SETTINGS_AUTO_SCREEN_OFF:
                            m_display_settings = MENU_DISPLAY_SETTINGS_AUTO_SCREEN_OFF_EN;
                            break;
                        case MENU_DISPLAY_SETTINGS_AUTO_SCREEN_OFF_DELAY:
                            m_display_settings = MENU_DISPLAY_SETTINGS_AUTO_SCREEN_OFF_DELAY_CHANGE;
                            break;
                        case MENU_DISPLAY_SETTINGS_BRIGHTNESS_MANUAL:
                            // if this is entered when auto display brightness is enabled, disable auto display brightness and set brightness to default
                            if (data->display_settings.is_auto_display_brightness_en) {
                                is_display_settings_changed = true;
                                data->display_settings.is_auto_display_brightness_en = false;
                                data->display_settings.display_brightness = DISPLAY_BRIGHTNESS_DEF;
                            }
                            m_display_settings = MENU_DISPLAY_SETTINGS_BRIGHTNESS_MANUAL_CHANGE;
                            break;
                        case MENU_DISPLAY_SETTINGS_BRIGHTNESS_AUTO:
                            m_display_settings = MENU_DISPLAY_SETTINGS_BRIGHTNESS_AUTO_EN;
                            break;
                        }

                        update_display = true;
                    }
                }
            }
            prev_btn = *btn;
        }

        // update the display
        if (update_display) {
            printf("m_display_settings=%d\r\n", m_display_settings);

            display_clear_screen();
            // display connection status indication if connected
            if (data->is_connected)
                display_wifi_icon(DISPLAY_COLOR);

            switch (m_display_settings) {
            case MENU_DISPLAY_SETTINGS_BRIGHTNESS:
                display_menu("Brightness", DISPLAY_COLOR, NULL, !DISPLAY_COLOR);
                break;
            case MENU_DISPLAY_SETTINGS_AUTO_SCREEN_OFF:
                display_menu("Auto", DISPLAY_COLOR, "Screen Off", DISPLAY_COLOR);
                break;
            case MENU_DISPLAY_SETTINGS_AUTO_SCREEN_OFF_DELAY:
                display_menu("Delay", DISPLAY_COLOR, NULL, !DISPLAY_COLOR);
                break;
            case MENU_DISPLAY_SETTINGS_BRIGHTNESS_MANUAL:
                display_menu("Manual", DISPLAY_COLOR, NULL, !DISPLAY_COLOR);
                break;
            case MENU_DISPLAY_SETTINGS_BRIGHTNESS_AUTO:
                display_menu("Auto", DISPLAY_COLOR, NULL, !DISPLAY_COLOR);
                break;
            case MENU_DISPLAY_SETTINGS_AUTO_SCREEN_OFF_EN:
                display_menu(data->display_settings.is_auto_screen_off_en ? "Enabled" : "Disabled", DISPLAY_COLOR, NULL, !DISPLAY_COLOR);
                break;
            case MENU_DISPLAY_SETTINGS_AUTO_SCREEN_OFF_DELAY_CHANGE:
                display_screen_timeout(data->display_settings.auto_screen_off_delay_sec, DISPLAY_COLOR);
                break;
            case MENU_DISPLAY_SETTINGS_BRIGHTNESS_MANUAL_CHANGE:
                display_brightness_val(data->display_settings.display_brightness, DISPLAY_COLOR);
                break;
            case MENU_DISPLAY_SETTINGS_BRIGHTNESS_AUTO_EN:
                display_menu(data->display_settings.is_auto_display_brightness_en ? "Enabled" : "Disabled", DISPLAY_COLOR, NULL, !DISPLAY_COLOR);
                break;
	    }

	        update_display = false;
	    }

        vTaskDelay(1 / portTICK_RATE_MS);
    }

    // exit with display on
    if (screen_off)
       display_on();

    if (is_display_settings_changed)
        set_data_to_storage(STORAGE_KEY_DISPLAY_SETTINGS, (void *) &data->display_settings, sizeof(display_settings_t));

    // return new mode
    return next_mode;
}

static app_mode_t menu_update(app_data_t *data) {
    int *btn = &(data->button_status);
    int prev_btn = *btn;

    // when changing mode, this task should be completed before starting the next mode
    bool exit = false;
    app_mode_t next_mode = data->mode;

    uint8_t m_update = MENU_UPDATE_STATUS;
    bool update_display = true;
    time_t btn_power_press_ms = 0;

    bool screen_off = false;
    time_t t_screen_on_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;

    //TODO: bool fw_update_available = is_fw_update_available();

    while (!exit) {
        if (*btn == prev_btn) {
            if (data->display_settings.is_auto_screen_off_en) {
                if (((xTaskGetTickCount() * portTICK_PERIOD_MS) - t_screen_on_ms) >= (data->display_settings.auto_screen_off_delay_sec * 1000)) {
                    display_off();
                    screen_off = true;
                }
            }

            if (!screen_off) {
                if (!((*btn >> BUTTON_POWER_BACK_STAT) & 0x01)) { // power button is pressed
                    int cur_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
                    if ((cur_ms - btn_power_press_ms) >= HEATER_OFF_LONG_PRESS_DUR_MS) {
                        next_mode = APP_MODE_STANDBY;
                        exit = true;
                        btn_power_press_ms = cur_ms;
                    }
                }
            }
        } else {
            if (data->display_settings.is_auto_screen_off_en) {
                t_screen_on_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
            }

            if (screen_off) {
                // enable screen only if all buttons are unpressed
                if (((*btn >> BUTTON_POWER_BACK_STAT) & 0x01) && ((*btn >> BUTTON_UP_STAT) & 0x01)
                    && ((*btn >> BUTTON_DOWN_STAT) & 0x01) && ((*btn >> BUTTON_TIMER_FORWARD_STAT) & 0x01)) {
                    screen_off = false;
                    display_on();
                }
            } else {
                if ((*btn & (1 << BUTTON_POWER_BACK_STAT)) != (prev_btn & (1 << BUTTON_POWER_BACK_STAT))) { // power button toggles
                    if ((*btn >> BUTTON_POWER_BACK_STAT) & 0x01) { // unpressed
                        switch (m_update) {
                        case MENU_UPDATE_TRIGGER_UPDATE:
                            exit = true;
                            break;
                        }

                        update_display = true;
                    } else {
                        btn_power_press_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
                    }
                } else if ((*btn & (1 << BUTTON_TIMER_FORWARD_STAT)) != (prev_btn & (1 << BUTTON_TIMER_FORWARD_STAT))) { // timer button toggles
                    if ((*btn >> BUTTON_TIMER_FORWARD_STAT) & 0x01) { // unpressed
                        switch (m_update) {
                        case MENU_UPDATE_STATUS:
                            m_update = MENU_UPDATE_TRIGGER_UPDATE;
                            break;
                        case MENU_UPDATE_TRIGGER_UPDATE:
                            //TODO: start_fw_update();
                            m_update = MENU_UPDATE_ONGOING_UPDATE;
                        }

                        update_display = true;
                    }
                }
            }
            prev_btn = *btn;
        }
        // update the display
        if (update_display) {
            update_display = false;
            printf("m_update=%d\r\n", m_update);

            display_clear_screen();
            // display connection status indication if connected
            if (data->is_connected)
                display_wifi_icon(DISPLAY_COLOR);

            switch (m_update) {
            case MENU_UPDATE_STATUS:
            #if 0 //TODO: 
                if (fw_update_available) {
                    //TODO: display_update_status(1, get_kaa_ota_release_key(), DISPLAY_COLOR);
                    vTaskDelay(DISPLAY_UPDATE_STATUS_FLASH_DURATION_MS / portTICK_RATE_MS);
                    m_update = MENU_UPDATE_TRIGGER_UPDATE;
                    update_display = true;
                } else {
            #endif
                    display_update_status(0, -1, DISPLAY_COLOR);
                    vTaskDelay(DISPLAY_UPDATE_STATUS_FLASH_DURATION_MS / portTICK_RATE_MS);
                    exit = true;
                //}
                break;
            case MENU_UPDATE_TRIGGER_UPDATE:
                display_update_trigger_msg(DISPLAY_COLOR);
                break;
            case MENU_UPDATE_ONGOING_UPDATE:
                display_update_installing_msg(DISPLAY_COLOR);
                break;
            }

            }

        vTaskDelay(1 / portTICK_RATE_MS);
    }

    // exit with display on
    if (screen_off)
       display_on();

    // return new mode
    return next_mode;
}

static void temp_sensor_task(void *param) {
    app_data_t *data = (app_data_t *) param;
    int *ambient_temp_c = &(data->ambient_temperature_celsius);
    int *temp_offset_c = &(data->ambient_temperature_offset_celsius);

    while(1) {
        *ambient_temp_c = tempsensor_get_temperature() + *temp_offset_c;
        printf("ambient_temp=%d\r\n", *ambient_temp_c);
        // printf(" \n IN Temp Sensor Task \n "); // Adding only for Testing_PS_26Aug2020

        vTaskDelay(TEMP_SENSOR_READ_INTERVAL_MS / portTICK_RATE_MS);
    }
}

static void light_sensor_task(void *param) {
    app_data_t *data = (app_data_t *) param;
    int val;
    int *ambient_light = &(data->ambient_light);

    while(1) {
        val = lightsensor_get_val();
        *ambient_light = (val > 100) ? 100 : val;
        printf("ambient_light=%d\r\n", *ambient_light);

        // printf(" \n IN Light Sensor Task \n "); // Adding only for Testing_PS_26Aug2020

        vTaskDelay(LIGHT_SENSOR_READ_INTERVAL_MS / portTICK_RATE_MS);
    }
}

#if 1
static void pilot_light_task(void *param) {
    app_data_t *data = (app_data_t *) param;
    app_mode_t *mode = &(data->mode);
    bool on = false;
    int *ambient_light = &(data->ambient_light), prev_ambient_light = -1;
    int *pilot_br = &(data->pilot_light_brightness);
    time_t t_set_brightness_ms = 0;
    bool prev_pilot_light_en_settings = false;

    while (1) {
        if (*mode == APP_MODE_STANDBY) {
            if (on) {
                pilot_light_off();
            }
            on = false;
        } else {
            if (data->settings.is_dim_pilot_light_en) {
                if (((xTaskGetTickCount() * portTICK_PERIOD_MS) - t_set_brightness_ms) >= DIM_PILOT_LIGHT_UPDATE_INTERVAL_MS) {
                    // set brightness based on ambient light
                    if (*ambient_light != prev_ambient_light) {
                        *pilot_br = *ambient_light * (AUTO_DIM_PILOT_LIGHT_MAX_BRIGHTNESS - AUTO_DIM_PILOT_LIGHT_MIN_BRIGHTNESS) / AUTO_DIM_PILOT_LIGHT_MAX_BRIGHTNESS + AUTO_DIM_PILOT_LIGHT_MIN_BRIGHTNESS;
                        pilot_light_set_brightness((uint8_t)*pilot_br);
                        t_set_brightness_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
                        printf("pilot light br=%d\r\n", *pilot_br);
                    }
                }
                prev_pilot_light_en_settings = true;
            } else {
                if (!on || prev_pilot_light_en_settings) {
                    pilot_light_on();
                }
                prev_pilot_light_en_settings = false;
            }
            on = true;
        }

        vTaskDelay(1 / portTICK_RATE_MS);
    }
}
#endif


static void night_light_task(void *param) {
    app_data_t *data = (app_data_t *) param;
    int *ambient_light = &(data->ambient_light), prev_ambient_light = -1;
    int *nlight_cfg = &(data->night_light_cfg);
    bool *nlight_auto_en = &(data->settings.is_night_light_auto_brightness_en);
    bool prev_nlight_auto_en = !(*nlight_auto_en);
    time_t t_set_brightness_ms = 0;

    while (1) {
        if (*nlight_auto_en) {
            // update only by interval
            if (!prev_nlight_auto_en || ((xTaskGetTickCount() * portTICK_PERIOD_MS) - t_set_brightness_ms) >= NIGHT_LIGHT_UPDATE_INTERVAL_MS) {
                // set brightness based on ambient light
                if (*ambient_light != prev_ambient_light) {
                    int nlight_br;
                    if (*ambient_light >= NIGHT_LIGHT_BRIGHTNESS_OFF_THS) {
                        nlight_br = 0;
                    } else {
                    // brighter in dark
                        nlight_br = NIGHT_LIGHT_BRIGHTNESS_MAX - (*ambient_light * (NIGHT_LIGHT_BRIGHTNESS_MAX - NIGHT_LIGHT_BRIGHTNESS_MIN) / NIGHT_LIGHT_BRIGHTNESS_MAX + NIGHT_LIGHT_BRIGHTNESS_MIN);
                    }
                    int r_br = nlight_br * GET_LED_R_VAL(*nlight_cfg) / 100;
                    int g_br = nlight_br * GET_LED_G_VAL(*nlight_cfg) / 100;
                    int b_br = nlight_br * GET_LED_B_VAL(*nlight_cfg) / 100;
                    // set night light color and brightness
                    night_light_set_br(r_br, g_br, b_br);

                    t_set_brightness_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
                    printf("night light %d %d %d\r\n", r_br, g_br, b_br);
                }
            }
            prev_nlight_auto_en = true;
        } else {
            if (prev_nlight_auto_en) {
                night_light_off();
            }

            prev_nlight_auto_en = false;
        }

        vTaskDelay(1 / portTICK_RATE_MS);
    }
}

#if 1
static void display_brightness_task(void *param) {
    app_data_t *data = (app_data_t *) param;
    int *ambient_light = &(data->ambient_light), prev_ambient_light = -1;
    int *display_brightness = &(data->display_settings.display_brightness), prev_display_brightness = -1;
    time_t t_set_brightness_ms = 0;

    while (1) {
        if(data->display_settings.is_auto_display_brightness_en) {
            if (((xTaskGetTickCount() * portTICK_PERIOD_MS) - t_set_brightness_ms) >= DISPLAY_AUTO_BRIGHTNESS_UPDATE_INTERVAL_MS) {
                // set brightness based on ambient light
                if (*ambient_light != prev_ambient_light) {
                    prev_ambient_light = *ambient_light;
                    *display_brightness = *ambient_light;
                    printf("change screen brightness br=%d\r\n", *display_brightness);
                    display_set_brightness(*display_brightness);
                    prev_display_brightness = *display_brightness;
                }

                t_set_brightness_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
            }
        } else {
            if (*display_brightness != prev_display_brightness) {
                printf("change screen brightness\r\n");
                // set screen brightness
                display_set_brightness(*display_brightness);
                prev_display_brightness = *display_brightness;
            }
        }

        vTaskDelay(1 / portTICK_RATE_MS);
    }
}
#endif

uint32_t waiting_results = 0;

esp_err_t ping_results(ping_target_id_t msgType, esp_ping_found * pf) {
    printf("AvgTime:%.1fmS Sent:%d Rec:%d Err:%d min(mS):%d max(mS):%d ", (float)pf->total_time/pf->recv_count, pf->send_count, pf->recv_count, pf->err_count, pf->min_time, pf->max_time );
    printf("Resp(mS):%d Timeouts:%d Total Time:%d\n",pf->resp_time, pf->timeout_count, pf->total_time);
    app_data->internet_conn = pf->send_count > pf->recv_count ? false: true;

    waiting_results = 0;
    return ESP_OK;
}

static void ping_task(void *param) {
    app_data_t *data = (app_data_t *) param;
    app_mode_t *mode = &(data->mode);
    ip4_addr_t ip = {
        .addr = 0x08080808
    }; // 8.8.8.8 or google.com
    uint32_t count = 1; // pings per report
    uint32_t timeout = 1; // seconds to consider timeout
    uint32_t delay = 1; // seconds between pings

    while (*mode == APP_MODE_DEBUG) {
	if (!waiting_results) {
            vTaskDelay(5000 / portTICK_PERIOD_MS);

	    esp_ping_set_target(PING_TARGET_IP_ADDRESS_COUNT, &count, sizeof(uint32_t));
            esp_ping_set_target(PING_TARGET_RCV_TIMEO, &timeout, sizeof(uint32_t));
            esp_ping_set_target(PING_TARGET_DELAY_TIME, &delay, sizeof(uint32_t));
            esp_ping_set_target(PING_TARGET_IP_ADDRESS, &ip.addr, sizeof(uint32_t));
            esp_ping_set_target(PING_TARGET_RES_FN, &ping_results, sizeof(ping_results));
            printf("\nPinging %s\n", inet_ntoa(ip));
            ping_init();
            waiting_results = 1;
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }

    vTaskDelete(NULL);
}

static int wifi_conn_stat(int stat) {
    printf("wifi_conn_stat stat=%d\r\n", stat);
    app_data->is_connected = stat ? true : false;
    display_wifi_icon(stat ? DISPLAY_COLOR : !DISPLAY_COLOR);
    return 0;
}


int app_set_mode(int mode) {
    if (app_data) {
        switch (mode) {
            case APP_MODE_STANDBY:
            case APP_MODE_MANUAL_TEMPERATURE:
            case APP_MODE_TIMER_INCREMENT:
            case APP_MODE_AUTO:
                app_data->mode = mode;
                break;
            default:
                return -1;
        }
        return 0;
    }

    return -1;
}


int app_get_mode(void) {
    if (app_data) {
        return app_data->mode;
    }

    return -1;
}


int app_get_ambient_temp(void) {
    if (app_data) {
        return app_data->ambient_temperature_celsius;
    }

    return 0x80000000;
}


int app_set_target_temp(int temp_c) {
    if (app_data) {
        if (temp_c >= TEMPERATURE_CELSIUS_VAL_MIN
            && temp_c <= TEMPERATURE_CELSIUS_VAL_MAX) {
            app_data->manual_temperature_celsius = temp_c;
            app_data->manual_temperature_fahrenheit = celsius_to_fahr(temp_c);

            // update target temperature in flash
            set_integer_to_storage(STORAGE_KEY_MANUAL_TEMP_CELSIUS, app_data->manual_temperature_celsius);
            set_integer_to_storage(STORAGE_KEY_MANUAL_TEMP_FAHRENHEIT, app_data->manual_temperature_fahrenheit);

            return 0;
        }
    }

    return -1;
}


int app_get_target_temp(void) {
    if (app_data) {
        return app_data->manual_temperature_celsius;
    }

    return 0x80000000;
}


int app_set_timer(int timer) {
    if (app_data) {
        if (timer >= 0
            && timer <= TIMER_MAX_VALUE_MINUTES) {
            app_data->current_timer_setting_min = timer;
            app_data->last_timer_setting_min = timer;
            set_integer_to_storage(STORAGE_KEY_LAST_TIMER_SETTING, timer);

            return 0;
        }
    }

    return -1;
}


int app_get_timer(void) {
    if (app_data) {
        return app_data->current_timer_setting_min;
    }

    return 0x80000000;
}


int app_activate_child_lock(bool en) {
    if (app_data) {
        app_data->is_child_lock_active = en;
        return 0;
    }

    return -1;
}


bool app_is_child_lock_activated(void) {
    if (app_data) {
        return app_data->is_child_lock_active;
    }

    return false;
}


int app_set_sched(void) {
    if (app_data) {
        return 0;
    }

    return -1;
}


int app_get_sched(void) {
    if (app_data) {
        return 0;
    }

    return -1;
}


int app_enable_autoset_time_date(bool en) {
    if (app_data) {
        bool *is_auto_time_date_en = &(app_data->is_auto_time_date_en);
        if (en != *is_auto_time_date_en) {
            *is_auto_time_date_en = en;

            if (!ntp_is_enabled() && en) 
                ntp_init(NTP_SERVER);
            else if (ntp_is_enabled() && en == false)
                ntp_deinit();

            // save new value to flash
            set_integer_to_storage(STORAGE_KEY_IS_AUTO_TIME_DATE_EN, (int) *is_auto_time_date_en);

            return 0;
        }
    }

    return -1;
}


bool app_is_autoset_time_date_enabled(void) {
    if (app_data) {
        return app_data->is_auto_time_date_en;
    }

    return false;
}


int app_enable_ap_mode(bool en) {
    if (app_data) {
        if (comm_wifi_dev) {
            if (en)
                comm_wifi_dev->wifi_ap_enable(comm_wifi_dev->wifi_ap_ssid, comm_wifi_dev->wifi_ap_pw);
            else
                comm_wifi_dev->wifi_ap_disable();

            return 0;
        }
    }

    return -1;
}


bool app_is_ap_mode_enabled(void) {
    if (app_data) {
        if (comm_wifi_dev) {
            return comm_wifi_dev->is_wifi_ap_enabled();
        }
    }

    return false;
}


int app_set_sta_mode_ssid(char *ssid, size_t len) {
    if (app_data) {
        set_string_to_storage(NVS_LUCIDTRON_SSID_KEY, ssid);
        return 0;
    }

    return -1;
}


int app_set_sta_mode_password(char *pw, size_t len) {
    if (app_data) {
        set_string_to_storage(NVS_LUCIDTRON_PW_KEY, pw);
        return 0;
    }

    return -1;
}


int app_enable_sta_mode(bool en) {
    if (app_data) {
        if (en) {
            if (comm_wifi_dev) {
                get_string_from_storage(NVS_LUCIDTRON_SSID_KEY, app_data->sta_ssid);
                get_string_from_storage(NVS_LUCIDTRON_PW_KEY, app_data->sta_pw);
                comm_wifi_dev->wifi_client_enable(app_data->sta_ssid, app_data->sta_pw);

                return 0;
            }
        }
    }

    return -1;
}


bool app_is_sta_mode_enabled(void) {
    if (app_data) {
        return true;
    }

    return false;
}



int app_set_temp_unit(int unit) {
    if (app_data) {
        if (unit == TEMP_UNIT_CELSIUS
            || unit == TEMP_UNIT_FAHRENHEIT) {
            app_data->settings.temperature_unit = unit;
            set_data_to_storage(STORAGE_KEY_SETTINGS, (void *) &app_data->settings, sizeof(settings_t));
            return 0;
        }
    }

    return -1;
}


int app_get_temp_unit(void) {
    if (app_data) {
        return app_data->settings.temperature_unit;
    }

    return -1;
}


int app_enable_autodim_pilot_light(bool en) {
    if (app_data) {
        bool *is_dim_pilot_light_en = &(app_data->settings.is_dim_pilot_light_en);
        if (en != *is_dim_pilot_light_en) {
            *is_dim_pilot_light_en = en;
            set_data_to_storage(STORAGE_KEY_SETTINGS, (void *) &app_data->settings, sizeof(settings_t));
        }
        return 0;
    }

    return -1;
}

bool app_is_autodim_pilot_light_enabled(void) {
    if (app_data) {
        return app_data->settings.is_dim_pilot_light_en;
    }

    return false;
}

int app_enable_night_light_auto_brightness(bool en) {
    if (app_data) {
        bool *is_nlight_auto_br_en = &(app_data->settings.is_night_light_auto_brightness_en);
        if (en != *is_nlight_auto_br_en) {
            *is_nlight_auto_br_en = en;
            set_data_to_storage(STORAGE_KEY_SETTINGS, (void *) &app_data->settings, sizeof(settings_t));
        }
        return 0;
    }

    return -1;
}

bool app_is_night_light_auto_brightness_enabled(void) {
    if (app_data) {
        return app_data->settings.is_night_light_auto_brightness_en;
    }

    return false;
}

int app_set_night_light_config(int cfg) {
    if (app_data) {
       int *nlight_cfg = &(app_data->night_light_cfg);
       // check if valid
#define GET_LED_R_VAL(BR) ((BR & LED_R_MASK) >> LED_R_POS)
       if (GET_LED_R_VAL(cfg) < NIGHT_LIGHT_BRIGHTNESS_MIN || GET_LED_R_VAL(cfg) > NIGHT_LIGHT_BRIGHTNESS_MAX)
           return -1;
       if (GET_LED_G_VAL(cfg) < NIGHT_LIGHT_BRIGHTNESS_MIN || GET_LED_G_VAL(cfg) > NIGHT_LIGHT_BRIGHTNESS_MAX)
           return -1;
       if (GET_LED_B_VAL(cfg) < NIGHT_LIGHT_BRIGHTNESS_MIN || GET_LED_B_VAL(cfg) > NIGHT_LIGHT_BRIGHTNESS_MAX)
           return -1;
       // set
       *nlight_cfg = cfg;
       // save
        set_integer_to_storage(STORAGE_KEY_NIGHT_LIGHT_CFG, cfg);
       return 0;
    }

    return -1;
}

int app_get_night_light_config(void) {
    if (app_data) {
        return app_data->night_light_cfg;
    }

    return -1;
}

int app_enable_child_lock(bool en) {
    if (app_data) {
        bool *is_child_lock_en = &(app_data->settings.is_child_lock_en);
        if (en != *is_child_lock_en) {
            *is_child_lock_en = en;
            set_data_to_storage(STORAGE_KEY_SETTINGS, (void *) &app_data->settings, sizeof(settings_t));
        }
        return 0;
    }

    return -1;
}

bool app_is_child_lock_enabled(void) {
    if (app_data) {
        return app_data->settings.is_child_lock_en;
    }

    return false;
}

int app_enable_autodim_display(bool en) {
    if (app_data) {
        bool *is_auto_display_brightness_en = &(app_data->display_settings.is_auto_display_brightness_en);
        if (en != *is_auto_display_brightness_en) {
            *is_auto_display_brightness_en = en;
            set_data_to_storage(STORAGE_KEY_DISPLAY_SETTINGS, (void *) &app_data->display_settings, sizeof(display_settings_t));
        }
        return 0;
    }

    return -1;
}

bool app_is_autodim_display_enabled(void) {
    if (app_data) {
        return app_data->display_settings.is_auto_display_brightness_en;
    }

    return false;
}

int app_set_screen_brightness(int br) {
    if (app_data) {
        if (br >= DISPLAY_BRIGHTNESS_MIN
            && br <= DISPLAY_BRIGHTNESS_MAX) {
            int *display_brightness = &(app_data->display_settings.display_brightness);
            if (br != *display_brightness) {
                *display_brightness = br;
                set_data_to_storage(STORAGE_KEY_DISPLAY_SETTINGS, (void *) &app_data->display_settings, sizeof(display_settings_t));
            }
            return 0;
        }
    }

    return -1;
}

int app_get_screen_brightness(void) {
    if (app_data) {
        return app_data->display_settings.display_brightness;
    }

    return 0x80000000;
}

int app_enable_auto_screen_off(bool en) {
    if (app_data) {
        bool *is_auto_screen_off_en = &(app_data->display_settings.is_auto_screen_off_en);
        if (en != *is_auto_screen_off_en) {
            *is_auto_screen_off_en = en;
            set_data_to_storage(STORAGE_KEY_DISPLAY_SETTINGS, (void *) &app_data->display_settings, sizeof(display_settings_t));
        }
        return 0;
    }

    return -1;
}

bool app_is_auto_screen_off_enabled(void) {
    if (app_data) {
        return app_data->display_settings.is_auto_screen_off_en;
    }

    return false;
}

int app_set_auto_screen_off_delay(int delay) {
    if (app_data) {
        if (delay >= AUTO_SCREEN_OFF_DELAY_SEC_MIN
            && delay <= AUTO_SCREEN_OFF_DELAY_SEC_MAX) {
            int *auto_screen_off_delay_sec = &(app_data->display_settings.auto_screen_off_delay_sec);
            if (delay != *auto_screen_off_delay_sec) {
                *auto_screen_off_delay_sec = delay;
                set_data_to_storage(STORAGE_KEY_DISPLAY_SETTINGS, (void *) &app_data->display_settings, sizeof(display_settings_t));
            }
            return 0;
        }
    }

    return -1;
}

int app_get_auto_screen_off_delay(void) {
    if (app_data) {
        return app_data->display_settings.auto_screen_off_delay_sec;
    }

    return 0x80000000;
}

int app_start_fw_update(void) {
    if (app_data) {
#if 0 //TODO:
        if (is_fw_update_available()) {
            start_fw_update();
            return 0;
        }
#endif
    }

    return -1;
}

int app_ota_start(char* loc)
{  
    return ota_start(loc);  
}

