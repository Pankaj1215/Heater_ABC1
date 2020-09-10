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

#ifndef __COMMUNICATION_SERVER_H_
#define __COMMUNICATION_SERVER_H_

//#define IOT_SERVER_ADDRESS  "ec2-54-149-179-29.us-west-2.compute.amazonaws.com"
#define IOT_SERVER_ADDRESS          "172.31.26.240"   // Original Line//



#define IOT_MSG_OUT                 1
#define IOT_MSG_IN                  2

#define WIFI_STAT_NONE              0
#define WIFI_STAT_READY             1
#define WIFI_STAT_CONFIG            2
#define WIFI_STAT_SLEEP             3
#define WIFI_STAT_ERROR             4

#define WIFI_AP_SSID                "Lucidtron_wifi"
//#define WIFI_CONFIG_TIMEOUT         180
#define WIFI_CONFIG_TIMEOUT         30

#define COMM_STAT_NONE              0
#define COMM_STAT_READY             1
#define COMM_STAT_ERROR             2
#define COMM_STAT_STARTING          3

#define VALUE_KEY                   "="

#define HTTP_CMD_SSID               "ssid"
#define HTTP_CMD_PW                 "password"


#define HTTP_CMD_FRESET             "factoryreset"
#define HTTP_CMD_RESET              "reset"
#define HTTP_CMD_WPS                "wps"

#define WIFI_AP_MODE_SSID_BASE "envi"
#define WIFI_AP_MODE_SSID_LEN 11


struct iot_message {
    int status;
    int msg_direction;
    char* message;
};

struct comm_wifi {
    int status;
    char wifi_ap_ssid[WIFI_AP_MODE_SSID_LEN + 1];
    char wifi_ap_pw[64];
    int (*initialize)(void);
    int (*send_msg)(char* destination, char* msgtosend, int lenght);
    int (*receive_msg)(char* toreceive, int lenght);
    int (*wps_enable)(void);
    int (*wps_disable)(void);
    int (*wifi_client_enable)(char* ssid, char* pw);
    int (*wifi_ap_enable)(char* ssid_ap, char* pw);
    int (*wifi_ap_disable)(void);
    _Bool (*is_wifi_ap_enabled)(void);
    unsigned char * (*wifi_ap_get_mac)(void);
    unsigned char * (*wifi_client_get_mac)(void);
    int (*wifi_scan)(void* variable_args);
    int (*wake_up)(void* param);
    int (*sleep)(void);
};

struct comm_bluetooth {
    int status;
    int (*initialize)(void);
    int (*send_msg)(char* destination, char* msgtosend, int lenght);
    int (*receive_msg)(char* toreceive, int lenght);
    int (*wake_up)(void* param);
    int (*sleep)(void* param);
};

struct comm_sigfox {
    int status;
    int (*initialize)(void);
    int (*send_msg)(char* destination, char* msgtosend, int lenght);
    int (*receive_msg)(char* toreceive, int lenght);
    int (*wake_up)(void* param);
    int (*sleep)(void* param);
};

struct comm_lora {
    int status;
    int (*initialize)(void);
    int (*send_msg)(char* destination, char* msgtosend, int lenght);
    int (*receive_msg)(char* toreceive, int lenght);
    int (*wake_up)(void* param);
    int (*sleep)(void* param);
};

struct comm_gsm {
    int status;
    int (*initialize)(void);
    int (*send_msg)(char* destination, char* msgtosend, int lenght);
    int (*receive_msg)(char* toreceive, int lenght);
    int (*wake_up)(void* param);
    int (*sleep)(void* param);
};

int get_iot_information(char* field, char* value);
int send_iot_information(char* field, char* value);

/*!
 * \fn int initialize_communication_service(void)
 * \brief initialize communication services and devices
 * \return returns success 
 */
int initialize_communication_service(void);

/*!
 * \fn struct comm_wifi* get_wifi_dev(void)
 * \brief get wifi device structure
 * \return returns success or fail if no wifi found
 */
struct comm_wifi* get_wifi_dev(void);

/*!
 * \fn void set_wifi_conn_status_change_cb(int (*cb)(int conn_stat))
 * \brief set wifi connection change callback
 * \param callback when wifi status was changed
 * \return returns success 
 */
void set_wifi_conn_status_change_cb(int (*cb)(int conn_stat));

/*!
 * \fn int ota_start(char* loc)
 * \brief run ota service
 * \param loc target location to download the file
 * \return returns success or fail if not found
 */
int ota_start(char* loc);

#endif //__COMMUNICATION_SERVER_H_
