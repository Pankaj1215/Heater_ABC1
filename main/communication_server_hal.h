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

#ifndef __COMMUNICATION_SERVER_HAL_H__
#define __COMMUNICATION_SERVER_HAL_H__

#define DEV_ERR_NONE                0
#define DEV_ERR_GSM                 1
#define DEV_ERR_SIGFOX              2
#define DEV_ERR_LORA                4
#define DEV_ERR_WIFI                8
#define DEV_ERR_BT                  16

#define WIFI_MAX_LEN                32

#define WIFI_SECURITY_NONE          0
#define WIFI_SECURITY_WPA_AES       1
#define WIFI_SECURITY_WPA_TKIP      2
#define WIFI_SECURITY_WPA_TKIP_AES  3
#define WIFI_SECURITY_WPA2          4
#define WIFI_SECURITY_WEP           5

struct wifi_config {
    int wifi_mode;
    int wifi_security;
    int wifi_protocol;
    char wifi_sta_ssid[WIFI_MAX_LEN];
    char wifi_sta_password[WIFI_MAX_LEN];
    int wifi_sta_bssid_set;
    int wifi_sta_bssid;
    int wifi_sta_channel;

    char wifi_ap_ssid[WIFI_MAX_LEN];
    char wifi_ap_password[WIFI_MAX_LEN];
    int wifi_ap_ssid_len;
    int wifi_ap_channel;
    int wifi_ap_authmode;
    int wifi_ap_ssid_hidden;
    int wifi_ap_max_connection;
    int wifi_ap_beacon_interval;
};

/*!
 * \fn int initilize_communication_devices()
 * \brief initialize the communication services and devices 
 * \return returns success 
 */
int initilize_communication_devices(void);

/*!
 * \fn void register_wifi_msg_callbk(int (*msg_callbk)(char* msg, char* ret_reply))
 * \brief wifi callback when REST msg received
 * \param msg_callback function pointer called
 * \param ret_reply storage string after calling callback
 * \return returns success 
 */
void register_wifi_msg_callbk(int (*msg_callbk)(char* msg, char* ret_reply));

/*!
 * \fn void register_wifi_handler(struct comm_wifi* cw)
 * \brief register the hal function from wifi core
 * \param cw structure pointer 
 * \return returns success 
 */
void register_wifi_handler(struct comm_wifi* cw);

/*!
 * \fn void register_wifi_conn_stat_callbk(int (*conn_stat_callbk)(int stat))
 * \brief callback function when wifi status changed
 * \param conn_stat_callbk function pointer called when status changed
 * \param stat status changed
 * \return returns success 
 */
void register_wifi_conn_stat_callbk(int (*conn_stat_callbk)(int stat));

void register_bt_handler(struct comm_bluetooth* cbt);
void register_sigfox_handler(struct comm_sigfox* cs);
void register_lora_handler(struct comm_lora* cl);
void register_gsm_handler(struct comm_gsm* cg);

#endif //__COMMUNICATION_SERVER_HAL_H__
