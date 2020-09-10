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

#ifndef __WIFI_CORE_H__
#define __WIFI_CORE_H__

#define ESP32_WIFI_UNKNOWN      0
#define ESP32_WIFI_CLIENT       1
#define ESP32_WIFI_AP           2
#define ESP32_WIFI_WPS          3
#define ESP32_WIFI_SLEEP        4
#define ESP32_ERROR             5

#define WEB_SVR_STAT_ERROR      -1
#define WEB_SVR_STAT_UNKNOWN    0
#define WEB_SVR_STAT_RUNNING    1
#define WEB_SVR_STAT_STOP       2

#define MAX_REPLY_BUF           1000
#define PARSER_KEY              "&"


#define P_TESTING     // Added only for Pankaj_ testing


/*!
 * \fn int wifi_version()
 * \brief returns the firmware wifi version
 * \return returns wifi version
 */
int wifi_version();

/*!
 * \fn int esp32_wifi_config(int mode, char* ssid, char* password)
 * \brief set the wifi configuration
 * \param mode wifi mode STA or AP
 * \param ssid name of the wifi to connect if STA, or ap name if AP
 * \param password password if STA, password of ap if mode is AP
 * \return returns success 
 */
int esp32_wifi_config(int mode, char* ssid, char* password);


/*!
 * \fn int esp32_initialise_wifi(void)
 * \brief initialize wifi device
 * \return always success
 */
int esp32_initialise_wifi(void);

int esp32_send_msg(char* destination, char* msgtosend, int lenght);
int esp32_receive_msg(char* toreceive, int lenght);

/*!
 * \fn int esp32_wps_enable(void)
 * \brief enable wps mode
 * \return returns success 
 */
int esp32_wps_enable(void);

/*!
 * \fn int esp32_wps_disable(void)
 * \brief disable wps mode
 * \return returns success 
 */
int esp32_wps_disable(void);

/*!
 * \fn int esp32_wifi_client_enable(char* ssid, char* pw)
 * \brief set wifi device to client mode
 * \param ssid wifi will connect to this ssid
 * \param mode pw will use this password to remote wifi ap
 * \return returns success 
 */
int esp32_wifi_client_enable(char* ssid, char* pw);

/*!
 * \fn int esp32_wifi_ap_enable(char* ssid_ap, char *pw)
 * \brief set wifi to ap mode
 * \param ssid_ap set the ap mode ssid
 * \param pw set the ap mode password
 * \return returns success 
 */
int esp32_wifi_ap_enable(char* ssid_ap, char *pw);

/*!
 * \fn int esp32_wifi_ap_disable(void)
 * \brief set wifi to ap mode to disable
 * \return returns success 
 */
int esp32_wifi_ap_disable(void);

/*!
 * \fn _Bool esp32_wifi_is_ap_enabled(void)
 * \brief check if wifi is in ap or not
 * \return return true if yes, falso if no
 */
_Bool esp32_wifi_is_ap_enabled(void);

/*!
 * \fn unsigned char *esp32_wifi_ap_get_mac(void)
 * \brief get the location of mac address string in ap mode
 * \return static address location of mac address
 */
unsigned char *esp32_wifi_ap_get_mac(void);

/*!
 * \fn unsigned char *esp32_wifi_client_get_mac(void)
 * \brief get the location of mac address string in client mode
 * \return static address location of mac address
 */
unsigned char *esp32_wifi_client_get_mac(void);

int esp32_wifi_scan(void* variable_args);
int esp32_wake_up(void* param);
int esp32_sleep(void);

/*!
 * \fn int esp32_reg_wifi_msg_callback(int (*msg_callbk)(char* msg, char* ret_reply))
 * \brief set a callback when wifi receive REST message
 * \param msg_callbk function pointer to be executed when wifi gets REST msg
 * \param ret_reply return string when message was handled
 * \return returns success 
 */
int esp32_reg_wifi_msg_callback(int (*msg_callbk)(char* msg, char* ret_reply));

/*!
 * \fn int esp32_reg_wifi_conn_callback(int (*wifi_conn_cb)(int conn_stat))
 * \brief sets the callback when wifi changed status
 * \param wifi_conn_cb function pointer to be executed when wifi stat changed
 * \param conn_stat wifi status
 * \return returns success 
 */
int esp32_reg_wifi_conn_callback(int (*wifi_conn_cb)(int conn_stat));

#endif //__WIFI_CORE_H__
