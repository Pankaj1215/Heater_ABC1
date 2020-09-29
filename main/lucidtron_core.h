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

#ifndef __LUCIDTRON_CORE_H__
#define __LUCIDTRON_CORE_H__

#include "communication_server.h"

/**
 *  This is a sample message
 *
 */

#define DEV_NO_ERR              0
#define DEV_ERR_BOOTUP          1
#define DEV_ERR_DEBUG           2
#define DEV_ERR_COMMSVR         4
#define DEV_ERR_PERISVR         8

#define MAX_LABEL_SIZE          50
#define MAX_LED_STAT            3
#define MAX_BASIC_SWITCH        5

#define MAX_STR_BUFF_SIZE       150

#define NVS_LUCIDTRON_SSID_KEY      "ssid_key"             // Original Lines
#define NVS_LUCIDTRON_PW_KEY        "password_key"         // Original Lines


// #define NVS_LUCIDTRON_SSID_KEY      "WF-Home"        //   Added only for Testing_PS27Aug2020
// #define NVS_LUCIDTRON_PW_KEY        "bksm1554"         //   Added only for Testing_PS27Aug2020
// #define NVS_LUCIDTRON_SSID_KEY      "moto"        //   Added only for Testing_PS27Aug2020
// #define NVS_LUCIDTRON_PW_KEY        "jamesBond"         //   Added only for Testing_PS27Aug2020



#define NVS_MQTT_BROKER             "mqtt_broker"
#define NVS_MQTT_USERNAME           "mqtt_username"
#define NVS_MQTT_PASSWORD           "mqtt_password"
#define NVS_MQTT_CHANID             "mqtt_chanid"

#define LUCIDTRON_UNIQUE_ID         "device11"
#define LUCIDTRON_SAMPLE_MACID      "112211224433"
#define LUCIDTRON_SAMPLE_PRODID     "eheat12345"

#define DEMO_MQTT_CRED              0

struct lucidtron_comm_device {
    void* wifi;
    void* lora;
    void* gsm;
    void* sigfox;
    void* bt;
};

struct lucidtron_peripheral_device {
    void* basic_switches[MAX_BASIC_SWITCH];
    void* lightsensor;
    void* thermalsensor;
    void* gps;
    void* acclerometer;
    void* humidity;
    void* pressure;
    void* vibration;
    void* ultrasonic;
    void* touch;            
    void* ecompass;
    void* audio_codec;
};

struct lucidtron_device {
    char firmware_ver[MAX_LABEL_SIZE];
    char product_identifier[MAX_LABEL_SIZE];
    char project_name[MAX_LABEL_SIZE];
    char device_info[MAX_LABEL_SIZE];
    void* device_led_status[MAX_LED_STAT];
    void* debug;
};

/*!
 * \fn int set_device_status(int status);
 * \brief set the status of the device
 * \param status the status to be set
 * \return returns success
 */
int set_device_status(int status);


#endif //__LUCIDTRON_CORE_H__
