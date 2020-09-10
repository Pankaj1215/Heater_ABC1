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

#include "communication_server.h"
#include "communication_server_hal.h"
#include "common.h"

#include "wifi_core.h"

int hal_initialize_gsm()
{

    return DEV_ERR_GSM;
}

int hal_initialize_lora()
{

    return DEV_ERR_LORA;
}

int hal_initialize_sigfox()
{

    return DEV_ERR_SIGFOX;
}

int hal_initialize_wifi()
{
    int ret;
    //initialize wifi then set to low power state
    ret = esp32_initialise_wifi();

    if(ret < 0)
        return DEV_ERR_WIFI;
    else 
        return DEV_ERR_NONE;
}

int hal_initialize_bluetooth()
{

    return DEV_ERR_BT;
}

int initilize_communication_devices()
{
    int ret = DEV_ERR_NONE;

    ret += hal_initialize_gsm();
    ret += hal_initialize_bluetooth();
    ret += hal_initialize_lora();
    ret += hal_initialize_sigfox();
    ret += hal_initialize_wifi();

    return ret;
}

void register_wifi_msg_callbk(int (*msg_callbk)(char* msg, char* ret_reply))
{
    esp32_reg_wifi_msg_callback(msg_callbk);
}

void register_wifi_conn_stat_callbk(int (*conn_stat_callbk)(int stat))
{
    esp32_reg_wifi_conn_callback(conn_stat_callbk);
}

void register_wifi_handler(struct comm_wifi* cw)
{
    cw->initialize          = &esp32_initialise_wifi;
    cw->send_msg            = &esp32_send_msg;
    cw->receive_msg         = &esp32_receive_msg;
    cw->wps_enable          = &esp32_wps_enable;
    cw->wps_disable         = &esp32_wps_disable;
    cw->wifi_client_enable  = &esp32_wifi_client_enable;
    cw->wifi_ap_enable      = &esp32_wifi_ap_enable;
    cw->wifi_ap_disable     = &esp32_wifi_ap_disable;
    cw->is_wifi_ap_enabled  = &esp32_wifi_is_ap_enabled;
    cw->wifi_ap_get_mac     = &esp32_wifi_ap_get_mac;
    cw->wifi_client_get_mac = &esp32_wifi_client_get_mac;
    cw->wifi_scan           = &esp32_wifi_scan;
    cw->wake_up             = &esp32_wake_up;
    cw->sleep               = &esp32_sleep;
}

void register_bt_handler(struct comm_bluetooth* cbt)
{

}

void register_sigfox_handler(struct comm_sigfox* cs)
{


}

void register_lora_handler(struct comm_lora* cl)
{

}

void register_gsm_handler(struct comm_gsm* cg)
{

}
