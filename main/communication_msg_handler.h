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

#ifndef __COMMUNICATION_MSG_HANDLER_H__
#define __COMMUNICATION_MSG_HANDLER_H__

/** 
 * mainflux message type
 *
 */
#define MSGTYPE_UNKNOWN                          0
#define MSGTYPE_SYSTEM                           1
#define MSGTYPE_SET                              2
#define MSGTYPE_GET                              3

/**
 * longhorn specific
 */
#define REMOTE_CMD_CMD_WHOAREYOU                 "WhoAreYou"
#define REMOTE_CMD_CMD_OWNDEVICE                 "OwnDevice"
#define REMOTE_CMD_CMD_HI                        "Hi"
#define REMOTE_CMD_CMD_HELLO                     "Hello"
#define REMOTE_CMD_EN_AP_MODE                    "en_ap_mode"
#define REMOTE_CMD_IS_AP_MODE_EN                 "is_ap_mode_en"
#define REMOTE_CMD_SET_STA_MODE_SSID             "set_sta_mode_ssid"
#define REMOTE_CMD_SET_STA_MODE_PASSWORD         "set_sta_mode_password"
#define REMOTE_CMD_EN_STA_MODE                   "en_sta_mode"
#define REMOTE_CMD_IS_STA_MODE_EN                "is_sta_mode_en"
#define REMOTE_CMD_GET_FW_VER                    "get_fw_version"
#define REMOTE_CMD_START_FW_UPDATE               "start_fw_update"
#define REMOTE_CMD_MQTT_BROKER                   "mqtt_broker"
#define REMOTE_CMD_MQTT_USERNAME                 "mqtt_username"
#define REMOTE_CMD_MQTT_PASSWORD                 "mqtt_password"
#define REMOTE_CMD_MQTT_CHANNEL                  "mqtt_channel"

#define REMOTE_CMD_CMD_STATUS                    "device_status"
#define REMOTE_CMD_GET_MODE                      "get_mode"
#define REMOTE_CMD_SET_MODE                      "set_mode"
#define REMOTE_CMD_GET_AMBIENT_TEMP              "get_ambient_temp"
#define REMOTE_CMD_SET_TARGET_TEMP               "set_target_temp"
#define REMOTE_CMD_GET_TARGET_TEMP               "get_target_temp"
#define REMOTE_CMD_SET_TIMER_SETTING             "set_timer_setting"
#define REMOTE_CMD_GET_TIMER_SETTING             "get_timer_setting"
#define REMOTE_CMD_ACTIVATE_CHILD_LOCK           "activate_child_lock"
#define REMOTE_CMD_IS_CHILD_LOCK_ACTIVATED       "is_child_lock_activated"
#define REMOTE_CMD_SET_SCHED                     "set_sched"
#define REMOTE_CMD_GET_SCHED                     "get_sched"
#define REMOTE_CMD_EN_AUTO_SET_TIME_DATE         "en_auto_set_time_date"
#define REMOTE_CMD_IS_AUTO_SET_TIME_DATE_EN      "is_auto_set_time_date_en"
#define REMOTE_CMD_SET_TEMP_UNIT                 "set_temp_unit"
#define REMOTE_CMD_GET_TEMP_UNIT                 "get_temp_unit"
#define REMOTE_CMD_EN_AUTO_DIM_PILOT_LIGHT       "en_auto_dim_pilot_light"
#define REMOTE_CMD_IS_AUTO_DIM_PILOT_LIGHT_EN    "is_auto_dim_pilot_light_en"
#define REMOTE_CMD_EN_NIGHT_LIGHT_AUTO_BRIGHTNESS "en_night_light_auto_brightness"
#define REMOTE_CMD_IS_NIGHT_LIGHT_AUTO_BRIGHTNESS_EN "is_night_light_auto_brightness_en"
#define REMOTE_CMD_SET_NIGHT_LIGHT_CONFIG        "set_night_light_config"
#define REMOTE_CMD_GET_NIGHT_LIGHT_CONFIG        "get_night_light_config"
#define REMOTE_CMD_EN_CHILD_LOCK                 "en_child_lock"
#define REMOTE_CMD_IS_CHILD_LOCK_EN              "is_child_lock_en"
#define REMOTE_CMD_EN_AUTO_DIM_DISPLAY           "en_auto_dim_display"
#define REMOTE_CMD_IS_AUTO_DIM_DISPLAY_EN        "is_auto_dim_display_en"
#define REMOTE_CMD_SET_SCREEN_BRIGHTNESS         "set_screen_brightness"
#define REMOTE_CMD_GET_SCREEN_BRIGHTNESS         "get_screen_brightness"
#define REMOTE_CMD_EN_AUTO_SCREEN_OFF            "en_auto_screen_off"
#define REMOTE_CMD_IS_AUTO_SCREEN_OFF_EN         "is_auto_screen_off_en"
#define REMOTE_CMD_SET_AUTO_SCREEN_OFF_DELAY_SEC "set_auto_screen_off_delay_sec"
#define REMOTE_CMD_GET_AUTO_SCREEN_OFF_DELAY_SEC "get_auto_screen_off_delay_sec"
#define REMOTE_CMD_OTA                           "ota"


/*!
 * \fn int mainflux_msg_handler(char* msg, char* response)
 * \brief format before calling the msg handler
 * \param msg raw message received
 * \param response string return after processing 
 * \return returns success
 */
int mainflux_msg_handler(char* msg, char* response);

/*!
 * \fn int aurora_msg_handler(char* msg, char* response);
 * \brief web server msg handler
 * \param msg raw message received
 * \param response string return after processing 
 * \return returns success
 */
int aurora_msg_handler(char* msg, char* response);


#endif //__COMMUNICATION_MSG_HANDLER_H__

