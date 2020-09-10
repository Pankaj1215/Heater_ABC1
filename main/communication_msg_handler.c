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
#include "esp_system.h"
#include "communication_server.h"
#include "communication_server_hal.h"
#include "communication_msg_handler.h"

#include "common.h"
//TODO: #include "debug.h"
#include "lucidtron_core.h"
#include "non_volatile_lib.h"
#include "sdk_util.h"
#include "app.h"
#include "mqtt.h"
#include "version.h"

#define KEYMARK_OPENCBRACKET        '{'
#define KEYMARK_QUOTE               '"'
#define KEYMARK_COLON               ':'
#define KEYMARK_COMMA               ','
#define KEYMARK_CLOSECBRACKET       '}'

struct comm_wifi* cwifi_dev;
struct comm_bluetooth* cbt_dev;
struct comm_sigfox* csig_dev;
struct comm_lora* clora_dev;
struct comm_gsm* cgsm_dev;

int message_label_value_handler(char* label, char* value, char* reply_buff);

int mainflux_msg_handler(char* msg, char* response)
{
    int i = 0;
    char label[100];
    char value[100];
    char* labelstart = NULL;
    char* labelend = NULL;
    char* valuestart = NULL;
    char* valueend = NULL;
    int testpass = 0;
    int quote_ctr = 0;
    int comma_ctr = 0;
    int colon_ctr = 0;
    char replybuff[500];
    int messagetype = MSGTYPE_UNKNOWN;
    int is_message_for_me = -1;
    char username[MAX_STR_BUFF_SIZE];

    printf("mainflux handler [%s]\n", msg);

    //checking msg validity
    while(*(msg+i) != 0)
    {
        if(*(msg+i) == KEYMARK_OPENCBRACKET)    //  {
            testpass++;
        if(*(msg+i) == KEYMARK_CLOSECBRACKET)   //  }
            testpass++;
        if(*(msg+i) == KEYMARK_QUOTE)           //  "
            quote_ctr++;
        if(*(msg+i) == KEYMARK_COLON)           //  :
            colon_ctr++;
        if(*(msg+i) == KEYMARK_COMMA)           //  ,
            comma_ctr++;
        i++;
    }
    if((quote_ctr % 2) != 0)
    {
        printf("missing quotes %d\n", quote_ctr);
        return FAIL;
    }
    else 
        testpass++;
    if((quote_ctr / 4) != colon_ctr)
    {
        printf("missing colon %d %d\n", quote_ctr, colon_ctr);
        return FAIL;
    }
    else 
        testpass++;
    if(colon_ctr != (comma_ctr+1))  //colon is +1 of comma
    {
        printf("missing comma %d %d\n", colon_ctr, comma_ctr);
        return FAIL;
    }
    else 
        testpass++;
    if(testpass == 5)
        printf("all test passed\n");

    //parsing the strings
    i = 0;
    while(*(msg+i) != 0)
    {
        switch(*(msg+i))
        {
            case KEYMARK_OPENCBRACKET:
                //printf("key openbracket\n");
                break;
            case KEYMARK_QUOTE:
                //printf("key quote\n");
                if(labelstart == NULL)
                {
                    labelstart = msg + i + 1;
                    memset(label, 0, 100);
                }
                else if(labelend == NULL)
                {
                    labelend = msg + i;
                }
                else if (valuestart == NULL)
                {
                    valuestart = msg + i + 1;
                    memset(value, 0, 100);
                }
                else if (valueend == NULL)
                {
                    valueend = msg + i;
                }
                break;
            case KEYMARK_COLON:
                //printf("key colon\n");
                break;
            case KEYMARK_COMMA:
            case KEYMARK_CLOSECBRACKET:

                strncpy(label, labelstart, labelend - labelstart); 
                strncpy(value, valuestart, valueend - valuestart); 
                printf("label [%s]\n", label);
                printf("value [%s]\n", value);

                if(strcmp(label, "cmd") == 0)
                {
                    if(strcmp(value, "system") == 0)
                        messagetype = MSGTYPE_SYSTEM;
                    else if(strcmp(value, "set") == 0)
                        messagetype = MSGTYPE_SET;
                    else if(strcmp(value, "get") == 0)
                        messagetype = MSGTYPE_GET;
                }

                if(strcmp(label, "target") == 0)
                {
                    memset(username, 0, MAX_STR_BUFF_SIZE); 
                    get_string_from_storage(NVS_MQTT_USERNAME, username);
                    if(strcmp(value, "all") == 0)
                        is_message_for_me = 0;
                    else if(strcmp(value, username) == 0)
                        is_message_for_me = 0;
                }

                //check if the required parameter is passed before proceeding
                if(messagetype != MSGTYPE_UNKNOWN)
                {
                    if(is_message_for_me != -1)
                    {
                        //TODO: call the msg handler callback here
                        memset(replybuff, 0, 500);
                        message_label_value_handler(label, value, replybuff);
                        printf("reply buff [%s]\n", replybuff);

                        if(strlen(replybuff) > 0)
                        {
                            printf("publishing message [%s]\n", replybuff);
                            mqtt_publish_message(replybuff, NULL);
                        }
                    }
                    else
                        printf("still waiting for correct target\n");
                }
                else 
                    printf("still waiting for messagetype\n");

                labelstart = NULL;
                labelend = NULL;
                valuestart = NULL;
                valueend = NULL;
                break;
        }
        i++;
    }

    return SUCCESS;
}

int aurora_msg_handler(char* msg, char* response)
{
    char* equalsign = NULL;
    char duplicate[MAX_STR_BUFF_SIZE];
    char label[MAX_STR_BUFF_SIZE];
    char value[MAX_STR_BUFF_SIZE];

    printf("msg=%s\r\n", msg);

    memset(label, 0, MAX_STR_BUFF_SIZE); 
    memset(value, 0, MAX_STR_BUFF_SIZE); 
    memset(duplicate, 0, MAX_STR_BUFF_SIZE); 

    strcpy(duplicate, msg);
    equalsign = strstr(duplicate, VALUE_KEY);
    if(equalsign != NULL) 
    {
        equalsign++;
        strcpy(value, equalsign);
        equalsign--;
        *equalsign = 0;
        strcpy(label, duplicate);
    }
    
    message_label_value_handler(label, value, response);

    return SUCCESS;
}

static char* str_replace(char* str, char* a, char* b)
{
    int len  = strlen(str);
    int lena = strlen(a), lenb = strlen(b);
    for (char* p = str; (p = strstr(p, a)); ++p) {
        if (lena != lenb) // shift end as needed
            memmove(p+lenb, p+lena, len - (p - str) + lenb);
        memcpy(p, b, lenb);
    }
    return str;
}

static void space_char_decode(char *str)
{
    str_replace(str, "%20", " ");
}

int message_label_value_handler(char* label, char* value, char* reply_buff)
{
    char broker[MAX_STR_BUFF_SIZE];
    char username[MAX_STR_BUFF_SIZE];
    char password[MAX_STR_BUFF_SIZE];
    char channel[MAX_STR_BUFF_SIZE];

    memset(broker, 0, MAX_STR_BUFF_SIZE); 
    memset(username, 0, MAX_STR_BUFF_SIZE); 
    memset(password, 0, MAX_STR_BUFF_SIZE); 
    memset(channel, 0, MAX_STR_BUFF_SIZE); 

    space_char_decode(value);

    //TODO: please revise if only, dont use if else if due to limit
    if (strcmp(label, "cmd") == 0) {
        if (strcmp(value, REMOTE_CMD_CMD_WHOAREYOU) == 0) {
            printf("REMOTE_CMD_CMD_WHOAREYOU \n");
            //sprintf(reply_buff, "%s\n%s\n%s", "Longhorn", "Private", "NotSure");
            sprintf(reply_buff, "%s", "Longhorn");
        } else if (strcmp(value, REMOTE_CMD_CMD_OWNDEVICE) == 0) {
            get_string_from_storage(NVS_MQTT_BROKER, broker);
            get_string_from_storage(NVS_MQTT_USERNAME, username);
            get_string_from_storage(NVS_MQTT_PASSWORD, password);
            get_string_from_storage(NVS_MQTT_CHANID, channel);
            sprintf(reply_buff, 
            "{\"broker\":\"%s\","
            "\"username\":\"%s\","
            "\"password\":\"%s\","
            "\"channel\":\"%s\"}", 
            broker,
            username,
            password,
            channel);
        } else if (strcmp(value, REMOTE_CMD_CMD_HI) == 0) {
            sprintf(reply_buff, "%s", "Hello");
        } else if (strcmp(value, REMOTE_CMD_CMD_HELLO) == 0) {
            sprintf(reply_buff, "%s", "Hi");
        }
    } else if (strcmp(label, REMOTE_CMD_EN_AP_MODE) == 0) {
        printf("REMOTE_CMD_EN_AP_MODE %s\r\n", value);
        app_enable_ap_mode(atoi(value));
    } else if (strcmp(label, REMOTE_CMD_IS_AP_MODE_EN) == 0) {
        printf("REMOTE_CMD_IS_AP_MODE_EN %s\r\n", value);
        sprintf(reply_buff, "%d", app_is_ap_mode_enabled());
    } else if (strcmp(label, REMOTE_CMD_SET_STA_MODE_SSID) == 0) {
        printf("REMOTE_CMD_SET_STA_MODE_SSID %s\r\n", value);
        app_set_sta_mode_ssid(value, strlen(value));
    } else if (strcmp(label, REMOTE_CMD_SET_STA_MODE_PASSWORD) == 0) {
        printf("REMOTE_CMD_SET_STA_MODE_PASSWORD %s\r\n", value);
        app_set_sta_mode_password(value, strlen(value));
    } else if (strcmp(label, REMOTE_CMD_EN_STA_MODE) == 0) {
        printf("REMOTE_CMD_EN_STA_MODE %s\r\n", value);
        app_enable_sta_mode(atoi(value));
    } else if (strcmp(label, REMOTE_CMD_IS_STA_MODE_EN) == 0) {
        printf("REMOTE_CMD_IS_STA_MODE_EN %s\r\n", value);
        sprintf(reply_buff, "%d", app_is_sta_mode_enabled());
    } else if (strcmp(label, REMOTE_CMD_GET_FW_VER) == 0) { 
        printf("REMOTE_CMD_GET_FW_VER\r\n");
        get_version(reply_buff);
    } else if (strcmp(label, REMOTE_CMD_START_FW_UPDATE) == 0) {
        printf("REMOTE_CMD_START_FW_UPDATE %s\r\n", value);
        app_start_fw_update();
    } else if (strcmp(label, REMOTE_CMD_MQTT_BROKER) == 0) {
        printf("REMOTE_CMD_MQTT_BROKER %s\r\n", value);
        set_string_to_storage(NVS_MQTT_BROKER, value);
        mqtt_set_broker(value);
        sprintf(reply_buff, "%s", "BROKER OK");
    } else if (strcmp(label, REMOTE_CMD_MQTT_USERNAME) == 0) {
        printf("REMOTE_CMD_MQTT_USERNAME %s\r\n", value);
        set_string_to_storage(NVS_MQTT_USERNAME, value);
        mqtt_set_username(value);
        sprintf(reply_buff, "%s", "USERNAME OK");
    } else if (strcmp(label, REMOTE_CMD_MQTT_PASSWORD) == 0) {
        printf("REMOTE_CMD_MQTT_PASSWORD %s\r\n", value);
        set_string_to_storage(NVS_MQTT_PASSWORD, value);
        mqtt_set_password(value);
        sprintf(reply_buff, "%s", "PASSWORD OK");
    } else if (strcmp(label, REMOTE_CMD_MQTT_CHANNEL) == 0) {
        printf("REMOTE_CMD_MQTT_CHANNEL %s\r\n", value);
        set_string_to_storage(NVS_MQTT_CHANID, value);
        mqtt_set_channel(value);
        sprintf(reply_buff, "%s", "CHANNEL OK");

    /* BELOW HERE ARE ALL LONGHORN SPECIFIC */

    } else if (strcmp(label, REMOTE_CMD_CMD_STATUS) == 0) {
        printf("REMOTE_CMD_GET_MODE %s\r\n", value);
        sprintf(reply_buff, 
            "{\"is_child_lock_en\":\"%d\","  
            "\"get_screen_brightness\":\"%d\"," 
            "\"get_target_temp\":\"%d\","
            "\"is_night_light_auto_brightness_en\":\"%d\","
            "\"get_night_light_config\":\"%d\","
            "\"get_timer_setting\":\"%d\","
            "\"get_mode\":\"%d\","
            "\"is_auto_set_time_date_en\":\"%d\""
            "}", 
            app_is_child_lock_enabled(),
            app_get_screen_brightness(),
            app_get_target_temp(),
            app_is_night_light_auto_brightness_enabled(),
            app_get_night_light_config(),
            app_get_timer(),
            app_get_mode(),
            app_is_autoset_time_date_enabled()
            );

    } else if (strcmp(label, REMOTE_CMD_GET_MODE) == 0) {
        printf("REMOTE_CMD_GET_MODE %s\r\n", value);
        sprintf(reply_buff, "%d", app_get_mode());
    } else if (strcmp(label, REMOTE_CMD_SET_MODE) == 0) {
        printf("REMOTE_CMD_SET_MODE %s\r\n", value);
        app_set_mode(atoi(value));
    } else if (strcmp(label, REMOTE_CMD_GET_AMBIENT_TEMP) == 0) {
        printf("REMOTE_CMD_GET_AMBIENT_TEMP %s\r\n", value);
        sprintf(reply_buff, "%d", app_get_ambient_temp());
    } else if (strcmp(label, REMOTE_CMD_SET_TARGET_TEMP) == 0) {
        printf("REMOTE_CMD_SET_TARGET_TEMP %s\r\n", value);
        app_set_target_temp(atoi(value));
    } else if (strcmp(label, REMOTE_CMD_GET_TARGET_TEMP) == 0) {
        printf("REMOTE_CMD_GET_TARGET_TEMP %s\r\n", value);
        sprintf(reply_buff, "%d", app_get_target_temp());
    } else if (strcmp(label, REMOTE_CMD_SET_TIMER_SETTING) == 0) {
        printf("REMOTE_CMD_SET_TIMER_SETTING %s\r\n", value);
        app_set_timer(atoi(value));
    } else if (strcmp(label, REMOTE_CMD_GET_TIMER_SETTING) == 0) {
        printf("REMOTE_CMD_GET_TIMER_SETTING %s\r\n", value);
        sprintf(reply_buff, "%d", app_get_timer());
    } else if (strcmp(label, REMOTE_CMD_ACTIVATE_CHILD_LOCK) == 0) {
        printf("REMOTE_CMD_ACTIVATE_CHILD_LOCK %s\r\n", value);
        app_activate_child_lock(atoi(value));
    } else if (strcmp(label, REMOTE_CMD_IS_CHILD_LOCK_ACTIVATED) == 0) {
        printf("REMOTE_CMD_IS_CHILD_LOCK_ACTIVATED %s\r\n", value);
        sprintf(reply_buff, "%d", app_is_child_lock_activated());
    } else if (strcmp(label, REMOTE_CMD_SET_SCHED) == 0) {
        printf("REMOTE_CMD_SET_SCHED %s\r\n", value);
    } else if (strcmp(label, REMOTE_CMD_GET_SCHED) == 0) {
        printf("REMOTE_CMD_GET_SCHED %s\r\n", value);
    } else if (strcmp(label, REMOTE_CMD_EN_AUTO_SET_TIME_DATE) == 0) {
        printf("REMOTE_CMD_EN_AUTO_SET_TIME_DATE %s\r\n", value);
        app_enable_autoset_time_date(atoi(value));
    } else if (strcmp(label, REMOTE_CMD_IS_AUTO_SET_TIME_DATE_EN) == 0) {
        printf("REMOTE_CMD_IS_AUTO_SET_TIME_DATE_EN %s\r\n", value);
        sprintf(reply_buff, "%d", app_is_autoset_time_date_enabled());
    } else if (strcmp(label, REMOTE_CMD_SET_TEMP_UNIT) == 0) {
        printf("REMOTE_CMD_SET_TEMP_UNIT %s\r\n", value);
        app_set_temp_unit(atoi(value));
    } else if (strcmp(label, REMOTE_CMD_GET_TEMP_UNIT) == 0) {
        printf("REMOTE_CMD_GET_TEMP_UNIT %s\r\n", value);
        sprintf(reply_buff, "%d", app_get_temp_unit());
    } else if (strcmp(label, REMOTE_CMD_EN_AUTO_DIM_PILOT_LIGHT) == 0) {
        printf("REMOTE_CMD_EN_AUTO_DIM_PILOT_LIGHT %s\r\n", value);
        app_enable_autodim_pilot_light(atoi(value));
    } else if (strcmp(label, REMOTE_CMD_IS_AUTO_DIM_PILOT_LIGHT_EN) == 0) {
        printf("REMOTE_CMD_IS_AUTO_DIM_PILOT_LIGHT_EN %s\r\n", value);
        sprintf(reply_buff, "%d", app_is_autodim_pilot_light_enabled());
    } else if (strcmp(label, REMOTE_CMD_EN_NIGHT_LIGHT_AUTO_BRIGHTNESS) == 0) {
        printf("REMOTE_CMD_EN_NIGHT_LIGHT_AUTO_BRIGHTNESS %s\r\n", value);
        app_enable_night_light_auto_brightness(atoi(value));
    } else if (strcmp(label, REMOTE_CMD_IS_NIGHT_LIGHT_AUTO_BRIGHTNESS_EN) == 0) {
        printf("REMOTE_CMD_IS_NIGHT_LIGHT_AUTO_BRIGHTNESS_EN %s\r\n", value);
        sprintf(reply_buff, "%d", app_is_night_light_auto_brightness_enabled());
    } else if (strcmp(label, REMOTE_CMD_SET_NIGHT_LIGHT_CONFIG) == 0) {
        printf("REMOTE_CMD_SET_NIGHT_LIGHT_CONFIG %s\r\n", value);
        app_set_night_light_config(atoi(value));
    } else if (strcmp(label, REMOTE_CMD_GET_NIGHT_LIGHT_CONFIG) == 0) {
        printf("REMOTE_CMD_GET_NIGHT_LIGHT_CONFIG %s\r\n", value);
        sprintf(reply_buff, "%d", app_get_night_light_config());
    } else if (strcmp(label, REMOTE_CMD_EN_CHILD_LOCK) == 0) {
        printf("REMOTE_CMD_EN_CHILD_LOCK %s\r\n", value);
        app_enable_child_lock(atoi(value));
    } else if (strcmp(label, REMOTE_CMD_IS_CHILD_LOCK_EN) == 0) {
        printf("REMOTE_CMD_IS_CHILD_LOCK_EN %s\r\n", value);
        sprintf(reply_buff, "%d", app_is_child_lock_enabled());
    } else if (strcmp(label, REMOTE_CMD_EN_AUTO_DIM_DISPLAY) == 0) {
        printf("REMOTE_CMD_EN_AUTO_DIM_DISPLAY %s\r\n", value);
        app_enable_autodim_display(atoi(value));
    } else if (strcmp(label, REMOTE_CMD_IS_AUTO_DIM_DISPLAY_EN) == 0) {
        printf("REMOTE_CMD_IS_AUTO_DIM_DISPLAY_EN %s\r\n", value);
        sprintf(reply_buff, "%d", app_is_autodim_display_enabled());
    } else if (strcmp(label, REMOTE_CMD_SET_SCREEN_BRIGHTNESS) == 0) {
        printf("REMOTE_CMD_SET_SCREEN_BRIGHTNESS %s\r\n", value);
        app_set_screen_brightness(atoi(value));
    } else if (strcmp(label, REMOTE_CMD_GET_SCREEN_BRIGHTNESS) == 0) {
        printf("REMOTE_CMD_GET_SCREEN_BRIGHTNESS %s\r\n", value);
        sprintf(reply_buff, "%d", app_get_screen_brightness());
    } else if (strcmp(label, REMOTE_CMD_EN_AUTO_SCREEN_OFF) == 0) {
        printf("REMOTE_CMD_EN_AUTO_SCREEN_OFF %s\r\n", value);
        app_enable_auto_screen_off(atoi(value));
    } else if (strcmp(label, REMOTE_CMD_IS_AUTO_SCREEN_OFF_EN) == 0) {
        printf("REMOTE_CMD_IS_AUTO_SCREEN_OFF_EN %s\r\n", value);
        sprintf(reply_buff, "%d", app_is_auto_screen_off_enabled());
    } else if (strcmp(label, REMOTE_CMD_SET_AUTO_SCREEN_OFF_DELAY_SEC) == 0) {
        printf("REMOTE_CMD_SET_AUTO_SCREEN_OFF_DELAY_SEC %s\r\n", value);
        app_set_auto_screen_off_delay(atoi(value));
    } else if (strcmp(label, REMOTE_CMD_GET_AUTO_SCREEN_OFF_DELAY_SEC) == 0) {
        printf("REMOTE_CMD_GET_AUTO_SCREEN_OFF_DELAY_SEC %s\r\n", value);
        sprintf(reply_buff, "%d", app_get_auto_screen_off_delay());
    } else if (strcmp(label, REMOTE_CMD_OTA) == 0) {
        printf("REMOTE_CMD_OTA%s\r\n", value);
        app_ota_start(value);
    } else {
        printf("unhandled label %s %s\r\n", label, value);
    }

    return SUCCESS;
}

