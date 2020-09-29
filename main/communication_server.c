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
#include "communication_server.h"
#include "communication_server_hal.h"
#include "communication_msg_handler.h"

#include "lucidtron_core.h"
#include "non_volatile_lib.h"
#include "common.h"
//#include "debug.h"
#include "sdk_util.h"
//#include "kaa_service.h"
#include "ota_update.h"
#include "mqtt.h"

// New Added _PS28Aug2020
#include "wifi_core.h"   // New Added for accessing macro #define P_TESTING which is placed in wifi_core.h



//overall status of communication side
int comm_device_status = DEV_ERR_NONE;

struct comm_wifi comm_wifi_dev;
struct comm_bluetooth comm_bt_dev;
struct comm_sigfox comm_sig_dev;
struct comm_lora comm_lora_dev;
struct comm_gsm comm_gsm_dev;

char ip[100];
char port[10];
char filename[255];
int ota_update_flag = 0;

int is_mqtt_running = 0;

void ota_update_thread(void* param)
{
    while(1)
    {
//        LOG_INFO("ip %s, port %s, filename %s\n", ip, port, filename);
        ota_update_start(ip, atoi(port), filename);

//        LOG_INFO("If you are seeing this message, ota update failed\n");
//        LOG_INFO("Retrying ota update\n");
        delay_milli(50000);
    }
}

int load_balancing_comm_devices(void)
{
    return ERROR;
}

int initialize_all_devices()
{
    //this verifies if the device was present
    comm_device_status = initilize_communication_devices();

    if(comm_device_status == DEV_ERR_NONE)
    {
        //LOG_INFO("Good all communications are ready!!!\n");
    }
    else if(comm_device_status == (DEV_ERR_GSM | DEV_ERR_LORA | 
        DEV_ERR_SIGFOX | DEV_ERR_WIFI | DEV_ERR_BT))
    {
        //LOG_EMERG("All communcations are down!!!\n");
        return ERROR;
    }

    if((comm_device_status & DEV_ERR_GSM) == 0)
        register_gsm_handler(&comm_gsm_dev);

    if((comm_device_status & DEV_ERR_SIGFOX) == 0)
        register_sigfox_handler(&comm_sig_dev);
        
    if((comm_device_status & DEV_ERR_LORA) == 0)
        register_lora_handler(&comm_lora_dev);

    if((comm_device_status & DEV_ERR_WIFI) == 0)
    {
        register_wifi_handler(&comm_wifi_dev);
        register_wifi_msg_callbk(aurora_msg_handler);
        mqtt_register_callback(mainflux_msg_handler);

        // set AP SSID in case the device enters AP mode
        // base the SSID on the last 3 digits of the AP MAC address prefixed with "<WIFI_AP_MODE_SSID_BASE>-"
        unsigned char *ap_mac = comm_wifi_dev.wifi_ap_get_mac();
        sprintf(comm_wifi_dev.wifi_ap_ssid, "%s-%02x%02x%02x", WIFI_AP_MODE_SSID_BASE, ap_mac[3], ap_mac[4], ap_mac[5]);
        // set to no ap password
        memset(comm_wifi_dev.wifi_ap_pw, 0, sizeof(comm_wifi_dev.wifi_ap_pw));
    }

    if((comm_device_status & DEV_ERR_BT) == 0)
        register_bt_handler(&comm_bt_dev);

    return SUCCESS;
}


//NOTE: returns
int event_handler_for_kaa(char* cmd, char* param)
{
//    LOG_INFO("cmd = %s, param = %s\n", cmd, param);
    printf("cmd = %s, param = %s\n", cmd, param);

    if(strcmp(cmd, "ota_update") == 0)
    {
        get_url(param, ip);
        get_port(param, port);
        get_filepath(param, filename);

        if(ota_update_flag == 0)
        {
            ota_update_flag = 1;
            create_thread(ota_update_thread, "ota_update_thread");
        }
    }

    return 0;
}



void wifi_handler(void* param)
{
    char wifi_ssid_config[MAX_STR_BUFF_SIZE];
    char wifi_pw_config[MAX_STR_BUFF_SIZE];
    int wifi_status = WIFI_STAT_NONE;
    int wifi_config_timeout = 0;

    char broker[MQTT_BUFF_LEN];
    char username[MQTT_BUFF_LEN];
    char password[MQTT_BUFF_LEN];
    char channel[MQTT_CHAN_LEN];

    printf("I am in wifi handler\n ");

//  set_string_to_storage(NVS_LUCIDTRON_SSID_KEY, "WF-Home");   // Added  Only For Testing _PS28Aug2020
//  set_string_to_storage(NVS_LUCIDTRON_PW_KEY, "bksm1554");    // Added only for Testing _PS28Aug2020

    // set_string_to_storage(NVS_LUCIDTRON_SSID_KEY, "moto");   // Added  Only For Testing _PS28Aug2020
    // set_string_to_storage(NVS_LUCIDTRON_PW_KEY, "Heyramramji");    // Added only for Testing _PS28Aug2020

    //LOG_INFO("%s running\n", __func__);
    //TODO: please revise this handler to match with wifi_core.c
    while(1)
    {

        #ifdef P_TESTING
        // Begin Testing Only...PS
         // comm_wifi_dev.wifi_client_enable("WF-Home", "bksm1554");

//        set_string_to_storage(NVS_LUCIDTRON_SSID_KEY, "WF-Home");   // Added  Only For Testing _PS28Aug2020
//         set_string_to_storage(NVS_LUCIDTRON_PW_KEY, "bksm1554");    // Added only for Testing _PS28Aug2020

       // set_string_to_storage(NVS_LUCIDTRON_SSID_KEY, "moto");   // Added  Only For Testing _PS28Aug2020
       // set_string_to_storage(NVS_LUCIDTRON_PW_KEY, "jamesBond");    // Added only for Testing _PS28Aug2020

        //Begin Testing Only...
       printf("\n Status of WiFi = %d\n", wifi_status );   // Added only for Testing _PS28Aug2020
        #endif
        switch(wifi_status)
        {
            case WIFI_STAT_NONE:
                //erase_storage_all();//this will erase all config
                //check if wifi has previous configuration
                //LOG_INFO("Wifi detected... checking for config\n");

                memset(wifi_ssid_config, 0, MAX_STR_BUFF_SIZE);

                get_string_from_storage(NVS_LUCIDTRON_SSID_KEY, wifi_ssid_config);

                if(strlen(wifi_ssid_config) > 0)
                {
                    wifi_status = WIFI_STAT_READY;
                    memset(wifi_pw_config, 0, MAX_STR_BUFF_SIZE);
                    get_string_from_storage(NVS_LUCIDTRON_PW_KEY, wifi_pw_config);
                    //set wifi to ap and start config
                    printf("wifi config found... connecting to ...\n");
                    printf("ssid=%s pw=%s\n", wifi_ssid_config, wifi_pw_config);

                    comm_wifi_dev.wifi_client_enable(wifi_ssid_config, wifi_pw_config);    // become station for mobile hotspot.
                }
                else
                {
                    wifi_status = WIFI_STAT_CONFIG;
                    //LOG_INFO("Config not found, starting as AP %s\n", WIFI_AP_SSID);
                    comm_wifi_dev.wifi_ap_enable(comm_wifi_dev.wifi_ap_ssid, comm_wifi_dev.wifi_ap_pw);   // become wifi for laptop .

                    printf("IN AP Mode wifi config found... connecting to ...\n");
                    printf("ssid=%s pw=%s\n", wifi_ssid_config, wifi_pw_config);

                }

                break;
            case WIFI_STAT_READY:
                break;
            case WIFI_STAT_CONFIG:
                break;
            case WIFI_STAT_SLEEP:
                break;
            case WIFI_STAT_ERROR:
                break;
            //default:
                //LOG_INFO("Invalid wifi code\n");
        }

        // Begin Testing Only...PS
        // comm_wifi_dev.wifi_client_enable("WF-Home", "bksm1554");    // To make Wifi AP , We can see that in Laptop Wifi
        //Begin Testing Only...

        //after initialization, its time to power up the devices
        //and proceed with their business logic

        //TODO: to fix this part: waiting for wifi to connect
        wifi_config_timeout++;
        if(wifi_config_timeout > WIFI_CONFIG_TIMEOUT)
        {
          //  printf("wifi config timeout,. attempting restart\n");  // Commented byPAug2020
            wifi_config_timeout = 0;
            if(is_mqtt_running == 0)
            {
            #if 0
                erase_string_in_storage(NVS_MQTT_BROKER);
                erase_string_in_storage(NVS_MQTT_USERNAME);
                erase_string_in_storage(NVS_MQTT_PASSWORD);
                erase_string_in_storage(NVS_MQTT_CHANID);
            #endif

               // get_string_from_storage(NVS_MQTT_BROKER, broker);   //only for testing Commented byPS_1Sept2020
               // printf("broker[0] = %d\n", broker[0]);    // // Commented byPAug2020
                if(strlen(broker)>0) 
                    mqtt_set_broker(broker);

                //get_string_from_storage(NVS_MQTT_USERNAME, username);    //only for testing Commented byPS_1Sept2020
                if(strlen(username)>0)
                    mqtt_set_username(username);

               // get_string_from_storage(NVS_MQTT_PASSWORD, password);   //only for testing Commented byPS_1Sept2020
                if(strlen(password)>0)
                    mqtt_set_password(password);

              //  get_string_from_storage(NVS_MQTT_CHANID, channel);    //only for testing Commented byPS_1Sept2020
                if(strlen(channel)>0)
                    mqtt_set_channel(channel);

              //  printf("xbroker [%s]\n", broker);                         // // Commented byPAug2020
              //  printf("xusername [%s]\n", username);                      // Commented byPAug2020
              //  printf("xpassword [%s]\n", password);                      // Commented byPAug2020
              //  printf("xchannel  [%s]\n", channel);                       // Commented byPAug2020
#if DEMO_MQTT_CRED
                printf("strlen(username) = %d\n", strlen(username));
                if(strlen(username) == 0)
                {
                    strcpy(username, MQTT_USERNAME);
                    mqtt_set_username(username);
                }
                if(strlen(password) == 0)
                {
                    strcpy(password, MQTT_PASSWORD);
                    mqtt_set_password(password);
                }
                if(strlen(channel) == 0) 
                {
                    strcpy(channel, MQTT_CHANID);
                    mqtt_set_channel(channel);
                }
#endif

                if((strlen(username)>0) &&
                   (strlen(password)>0) &&
                   (strlen(channel)>0))
                {
                    is_mqtt_running = 1;
                    create_thread_with_stackvalue(
                        mqtt_init_service, 
                        "mqtt_init_service",
                        20000
                        );
                }  
            }
        }
        delay_milli(1000);
    }
}



void bluetooth_handler(void* param)
{
    while(1)
    {
        //LOG_INFO("%s running\n", __func__);
        delay_milli(60000);
    }
}

void gsm_handler(void* param)
{
    while(1)
    {
        //LOG_INFO("%s running\n", __func__);
        delay_milli(60000);
    }
}

void sigfox_handler(void* param)
{
    while(1)
    {
        //LOG_INFO("%s running\n", __func__);
        delay_milli(60000);
    }
}

void lora_handler(void* param)
{
    while(1)
    {
        //LOG_INFO("%s running\n", __func__);
        delay_milli(60000);
    }
}

//responsble for queueing  the message, will send and receive 
//when available, 
void poll_iot_information(void* param)
{
    //TODO: erase this for debug only
    //set_string_to_storage(NVS_LUCIDTRON_SSID_KEY, "lucidtron globe");
    //set_string_to_storage(NVS_LUCIDTRON_PW_KEY, "iotdesign");

    //LOG_INFO("%s is running...\n", __func__);

    if((comm_device_status & DEV_ERR_GSM) == 0)
        create_thread(gsm_handler, "gsm_handler");

    if((comm_device_status & DEV_ERR_SIGFOX) == 0)
        create_thread(sigfox_handler, "sigfox_handler");
        
    if((comm_device_status & DEV_ERR_LORA) == 0)
        create_thread(lora_handler, "lora_handler");

    if((comm_device_status & DEV_ERR_WIFI) == 0)
        create_thread(wifi_handler, "wifi_handler");

    if((comm_device_status & DEV_ERR_BT) == 0)
        create_thread(bluetooth_handler, "bluetooth_handler");

    //initialize kaa service
    delay_milli(15000); //connect first before initializing kaa
    //create_thread_with_stackvalue((void *)kaa_main, "kaa_main", 100000);
    //register_event_handler(event_handler_for_kaa);

    //this will poll or update the status of each devs
    while(1)
    {
        delay_milli(10000);
    }
}

/********** PUBLIC INTERFACE *******************/

//queue
int send_iot_information(char* field, char* value)
{

    return ERROR;
}
int get_iot_information(char* field, char* value)
{

    return ERROR;
}

int initialize_communication_service(void)
{
    int ret; 

    ret = initialize_all_devices();
    if(ret != ERROR)
    {
        create_thread(poll_iot_information, "poll_iot_information");
    }
    else if (ret == ERROR)
        return ERROR;

    return SUCCESS;
}

void set_wifi_conn_status_change_cb(int (*cb)(int conn_stat)) {
    register_wifi_conn_stat_callbk(cb);
}

struct comm_wifi* get_wifi_dev(void)
{
    return &comm_wifi_dev;
}
struct comm_bluetooth* get_bluetooth_dev(void)
{
    return &comm_bt_dev;
}
struct comm_sigfox* get_sig_dev(void)
{
    return &comm_sig_dev;
}
struct comm_lora* get_lora_dev(void)
{
    return &comm_lora_dev;
}
struct comm_gsm* get_gsm_dev(void)
{
    return &comm_gsm_dev;
}

int ota_start(char* loc)
{
//    LOG_INFO("cmd = %s, param = %s\n", cmd, param);
    printf("%s loc=%s\n", __func__, loc);

    get_ip(loc, ip);
    printf("ip=%s\r\n", ip);
    get_port(loc, port);
    printf("port=%s\r\n", port);
    get_filepath(loc, filename);
    printf("filename=%s\r\n", filename);

    if(ota_update_flag == 0)
    {
        ota_update_flag = 1;
        create_thread(ota_update_thread, "ota_update_thread");
    }

    return 0;
}

