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

/* HTTP GET Example using plain POSIX sockets

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_wps.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/api.h"

#include "non_volatile_lib.h" 
#include "lucidtron_core.h"
#include "wifi_core.h"

// #include "thing_shadow_sample.c"  // new added for connected bit // p14Sept20


#ifdef P_TESTING

// #define thing_Shadow   // for define thing shadow
#ifdef thing_Shadow
#include "aws_iot_config.h"
#include "aws_iot_log.h"
#include "aws_iot_version.h"
#include "aws_iot_mqtt_client_interface.h"
#include "aws_iot_shadow_interface.h"
#endif

#define subscribePublishTopic
#ifdef subscribePublishTopic
#include "aws_iot_config.h"
#include "aws_iot_log.h"
#include "aws_iot_version.h"
#include "aws_iot_mqtt_client_interface.h"
#include "aws_iot_shadow_interface.h"

#include "common.h"  // new added for connected bit // p17Sept20

unsigned char uchTopic_Set_temp_subscribe_status = FALSE;
unsigned char uchTopic_HeaterParameter_Publish_status = TRUE;  // By Default publish Heater Parameter

#endif

// #define HTTP_CLIENT_Code

#include "esp_http_client.h"    // New Added Header Files for HTTP File..28Aug2020//
 // #define DATA_HTTP_BIN_ORG                               // For Showing Data On httpbin.org
  //  #define WORKING_DATA_ON_THINK_SPEAK_SERVER           // For Showing data on Thinkspeak server
   // #define WORKING_AWS_HTTP_GET_POST                    // FOr Get POSt HTTP_working_Backup_function
    #define ONLY_FOR_TESTING_DATA_SERVER                // For Testing TalkBack feature of ThinkSpeak for Get Data.

// #define HTTP_POST_FUNCTION_PRADEEP_SIR    // Testing of Function HTTP_POST_Function  by Praddep SIR


/* Constants that aren't configurable in menuconfig */
	#define WEB_SERVER              "192.168.43.190"        //    The IP Address of ESP32 : 192.168.43.190 in my SSID Netwoek WF-Home
	#define WEB_PORT                "3000"
	#define WEB_URL                 "http://192.168.43.190/"
	#define WEB_SERVER_PORT         80
#else                                                          // Original Code WEB configure Lines of Last working code given by Client.
/* Constants that aren't configurable in menuconfig */
	 #define WEB_SERVER              "192.168.254.127"
	#define WEB_PORT                "3000"
	#define WEB_URL                 "http://192.168.254.127/"
	 #define WEB_SERVER_PORT         80
#endif


static EventGroupHandle_t wifi_event_group;
const int CONNECTED_BIT = BIT0;

const static char http_html_header[] =
    "HTTP/1.1 200 OK\r\nContent-type: text/html\r\n\r\n";
const static char http_html_reply[] =
    "<!DOCTYPE html>"
    "lucidtron no reply\r\n\r\n";

void http_web_server();

int (*wifi_msg_handler_callback)(char* msg, char* ret_reply) = NULL;
int (*web_server_msg_handler)(char* msg, char* response, int res_len) = NULL;
int (*wifi_conn_stat_callback)(int conn_stat) = NULL;
int esp32_wifi_status = ESP32_WIFI_UNKNOWN;
int web_server_status = WEB_SVR_STAT_UNKNOWN;
wifi_config_t global_wifi_config;

static uint8_t ap_mac[6];
static uint8_t sta_mac[6];

static bool wifi_ap_en = false;

void run_msg_handler_callback(void* param)
{
    while(1)
    {
        printf("running msg handler thread\n");
        http_web_server();
    }
}

int esp32_reg_wifi_msg_callback(int (*wifi_callback)(char* msg, char* ret_reply))
{
    if(wifi_callback == NULL)
        return -1;

    wifi_msg_handler_callback = wifi_callback;
    return 0;
}

int esp32_reg_wifi_conn_callback(int (*wifi_conn_cb)(int conn_stat)) {
    if (wifi_conn_cb == NULL)
        return -1;

    wifi_conn_stat_callback = wifi_conn_cb;

    return 0;
}



//TODO: improve label value checking, this may cause hangup if wrong command
void http_web_server()
{
    struct netconn *conn, *newconn;
    err_t err, err1;

    struct netbuf* inbuf;
    char* buff;
    char* msg_mark;
    u16_t buflen;

    char* marker;
    char* marker_last = NULL;
    char reply_buff[MAX_REPLY_BUF];

    if(web_server_status == WEB_SVR_STAT_RUNNING)
        return;
    web_server_status = WEB_SVR_STAT_RUNNING;

    conn = netconn_new(NETCONN_TCP);
    netconn_bind(conn, NULL, WEB_SERVER_PORT);
    netconn_listen(conn);
    do{
        printf("%s %d\n", __func__, __LINE__);
        err = netconn_accept(conn, &newconn);
        if(err == ERR_OK)
        {
            printf("%s %d\n", __func__, __LINE__);
            err1 = netconn_recv(newconn, &inbuf);
            if(err1 == ERR_OK)
            {
                printf("%s %d\n", __func__, __LINE__);
                //accept msg and apply 0 end to string
                netbuf_data(inbuf, (void**)&buff, &buflen);
                *(buff+buflen) = 0;
                printf("%s %d buff -->%s<-- %d \n", __func__, __LINE__, buff, buflen);

                if(strstr(buff, "POST") != NULL)
                {
                    msg_mark = strstr(buff, "\r\n\r\n");
                    if (msg_mark)
                        msg_mark += strlen("\r\n\r\n");
                }
                else //if(strstr(buff, "GET") != NULL)
                {
                    //orig
                    //msg_mark = strstr(buff, "\r\n\r\n");
                    //msg_mark += strlen("\r\n\r\n");

                    msg_mark = strstr(buff, "?");
                    if (msg_mark) {
                        msg_mark++;
                        printf("get msg [%s]\n", msg_mark);
                        marker = strstr(msg_mark, " ");
                        if(marker == NULL)
                            printf("marker null\n");
                        else
                            *marker = 0;
                    }
                }

                if(msg_mark != NULL)
                {
                    printf("%s %d\n", __func__, __LINE__);
                    printf("data received\n-->%s<--\n", msg_mark);
                    //msg parser
                    do
                    {
                        marker = msg_mark;
                        while(marker != NULL)
                        {
                            printf("%s %d\n", __func__, __LINE__);
                            printf("marker [%s]\n", marker);
                            marker_last = marker;
                            marker = strstr(marker, PARSER_KEY);
                            if(marker != NULL) marker++;
                        }
                        printf("%s %d\n", __func__, __LINE__);
                        printf("last marker [%s]\n", marker_last);
                        if(strlen(marker_last) > 0)
                        {
                            //this should be the message handler
                            //TODO: please check the lenght of reply
                            if(wifi_msg_handler_callback != NULL)
                            {
                                printf("%s %d\n", __func__, __LINE__);
                                memset(reply_buff, 0, MAX_REPLY_BUF);
                                wifi_msg_handler_callback(marker_last, reply_buff);
                            }

                            printf("%s %d\n", __func__, __LINE__);
                            if(msg_mark == marker_last) break;
                            marker_last--;
                            *marker_last = 0;
                        }
                    }while(1);
                    //msg parser
                }
                //reply to msg
                printf("%s %d\n", __func__, __LINE__);
                netconn_write(newconn, http_html_header,
                        sizeof(http_html_header)-1, NETCONN_NOCOPY);
                if(strlen(reply_buff) > 0)
                {
                    printf("%s %d\n", __func__, __LINE__);
                    netconn_write(newconn, reply_buff,
                            strlen(reply_buff), NETCONN_NOCOPY);
                }
                else
                {
                    printf("%s %d\n", __func__, __LINE__);
                    netconn_write(newconn, http_html_reply,
                            sizeof(http_html_reply)-1, NETCONN_NOCOPY);
                }
                printf("%s %d\n", __func__, __LINE__);
                netconn_close(newconn);
                netbuf_delete(inbuf);
            }
            printf("%s %d\n", __func__, __LINE__);
            netconn_delete(newconn);
        }
    }while(err == ERR_OK);
    netconn_close(conn);
    netconn_delete(conn);
}




//  Commented for testing
//int event_handler(void *ctx, system_event_t *event)
//{
//    switch(event->event_id) {
//        //deprecated
//        case SYSTEM_EVENT_WIFI_READY:
//        case SYSTEM_EVENT_SCAN_DONE:
//            break;
//            //deprecated
//
//            //WIFI_MODE_STA
//        case SYSTEM_EVENT_STA_START:
//            esp_wifi_connect();
//            break;
//        case SYSTEM_EVENT_STA_STOP:
//            break;
//        case SYSTEM_EVENT_STA_CONNECTED:
//            if (wifi_conn_stat_callback)
//                wifi_conn_stat_callback(1);
//            break;
//        case SYSTEM_EVENT_STA_DISCONNECTED:
//            /* This is a workaround as ESP32 WiFi libs don't currently
//               auto-reassociate. */
//            printf("disconnected\n");
//            esp_wifi_connect();
//            xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
//            if (wifi_conn_stat_callback)
//                wifi_conn_stat_callback(0);
//            break;
//        case SYSTEM_EVENT_STA_AUTHMODE_CHANGE:
//            break;
//        case SYSTEM_EVENT_STA_GOT_IP:
//            xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
//            printf("connected\n");
//            break;
//            //case SYSTEM_EVENT_AP_STA_GOT_IP:  //no macro like this TODO
//            //break;
//
//            //WIFI_MODE_AP
//        case SYSTEM_EVENT_AP_START:
//            wifi_ap_en = true;
//            break;
//        case SYSTEM_EVENT_AP_STOP:
//            wifi_ap_en = false;
//            break;
//        case SYSTEM_EVENT_AP_STACONNECTED:
//            if (wifi_conn_stat_callback)
//                wifi_conn_stat_callback(1);
//            break;
//        case SYSTEM_EVENT_AP_STADISCONNECTED:
//            if (wifi_conn_stat_callback)
//                wifi_conn_stat_callback(0);
//            break;
//        case SYSTEM_EVENT_AP_PROBEREQRECVED:
//            break;
//
//            //WIFI WPS
//        case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:
//            esp_wifi_wps_disable();
//            // save SSID and password
//            memset(&global_wifi_config, 0, sizeof(global_wifi_config));
//            esp_wifi_get_config(WIFI_IF_STA, &global_wifi_config);
//            set_string_to_storage(NVS_LUCIDTRON_SSID_KEY, (char *)(global_wifi_config.sta.ssid));
//            set_string_to_storage(NVS_LUCIDTRON_PW_KEY, (char *)(global_wifi_config.sta.password));
//            esp_wifi_connect();
//            break;
//        case SYSTEM_EVENT_STA_WPS_ER_FAILED:
//            break;
//        case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:
//            break;
//        case SYSTEM_EVENT_STA_WPS_ER_PIN:
//            break;
//
//        default:
//            break;
//    }
//    return ESP_OK;
//}





int esp32_initialise_wifi(void)
{
    //this init the high level protocol handler for the driver
    tcpip_adapter_init();
    wifi_event_group = xEventGroupCreate();

    //prepare the event callback  // Commented for testing_P16Sept2020_TBUC
   // ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );

    //this initialize the wifi driver 
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );

    esp32_wifi_status = ESP32_WIFI_SLEEP;

    if(web_server_status == WEB_SVR_STAT_UNKNOWN)
    {
        xTaskCreate(run_msg_handler_callback,
                "run_msg_handler_callback", 8192, NULL, 5, NULL);
    }

    return 0;
}


int esp32_wifi_config(int mode, char* ssid, char* password)
{
    //this config will save the config in ram, theres also an option to
    //save in nvs flash, that means even when restart, config are stored
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );

    //WIFI_MODE_NULL
    //WIFI_MODE_STA
    //WIFI_MODE_AP
    //WIFI_MODE_APSTA
    ESP_ERROR_CHECK( esp_wifi_set_mode(mode) );
    if(mode == WIFI_MODE_STA)
    {
        memset(&global_wifi_config, 0, sizeof(global_wifi_config));

        strncpy((char*)global_wifi_config.sta.ssid, ssid, 32);
        strncpy((char*)global_wifi_config.sta.password, password, 64);

        ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &global_wifi_config) );
    }
    else if(mode == WIFI_MODE_AP)
    {
        /*
        tcpip_adapter_dhcpc_stop(TCPIP_ADAPTER_IF_AP);
        tcpip_adapter_ip_info_t ipInfo;
        IP4_ADDR(&ipInfo.ip, 192,168,100,99);
        IP4_ADDR(&ipInfo.gw, 192,168,100,1);
        IP4_ADDR(&ipInfo.netmask, 255,255,255,0);
        tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_AP, &ipInfo);
        */

        memset(&global_wifi_config, 0, sizeof(global_wifi_config));

        strncpy((char*)global_wifi_config.ap.ssid, ssid, 32);
        strncpy((char*)global_wifi_config.ap.password, password, 64);
        global_wifi_config.ap.channel        = 0;
        if (strlen(password) == 0) {
            global_wifi_config.ap.authmode   = WIFI_AUTH_OPEN;
        } else {
            global_wifi_config.ap.authmode   = WIFI_AUTH_WPA_WPA2_PSK;
        }
        global_wifi_config.ap.ssid_hidden    = 0;
        global_wifi_config.ap.max_connection = 1;
        global_wifi_config.ap.beacon_interval= 100;

        esp_wifi_set_mode(WIFI_MODE_AP);
            
        ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_AP, &global_wifi_config) );
    }

    //ESP_ERROR_CHECK( esp_wifi_start() );
    return 0;
}

int esp32_wake_up(void* param)
{

    return -1;
}
int esp32_sleep(void)
{

    return -1; 
}



int esp32_send_msg(char* destination, char* msgtosend, int lenght)
{


    return -1;
}


int esp32_receive_msg(char* toreceive, int lenght)
{


    return -1;
}



int esp32_wifi_scan(void* variable_args)
{

    return -1;
}
//wifi mode config
int esp32_wps_enable(void)
{
    esp_wps_config_t config = WPS_CONFIG_INIT_DEFAULT(WPS_TYPE_PBC);

    //if(esp32_wifi_status == ESP_IF_WIFI_AP) return -1;
    if(esp32_wifi_status != ESP32_WIFI_WPS)
    {
        esp32_wifi_status = ESP32_WIFI_WPS;
        esp_wifi_stop();
        esp_wifi_set_mode(WIFI_MODE_STA);
        esp_wifi_start();

        esp_wifi_wps_enable(&config);
        esp_wifi_wps_start(0);
    }

    return 0;
}
int esp32_wps_disable(void)
{
    esp_wifi_wps_disable();
    return 0;
}
int esp32_wifi_client_enable(char* ssid, char* pw)
{
    esp_wifi_stop();
    esp32_wifi_status = ESP32_WIFI_CLIENT;
    esp32_wifi_config(WIFI_MODE_STA, ssid, pw);
    esp_wifi_start();

    return 0;
}
int esp32_wifi_ap_enable(char* ssid_ap, char *pw)
{
    //do not call this, this will erase existing config
    //esp_wifi_stop();
    esp32_wifi_status = ESP32_WIFI_AP;
    esp32_wifi_config(WIFI_MODE_AP, ssid_ap, pw);
    esp_wifi_start();

    return 0;
}

int esp32_wifi_ap_disable(void) {
    if(esp32_wifi_status == ESP32_WIFI_AP) {
        //esp32_wifi_config(WIFI_MODE_NULL, NULL, NULL); 
        esp_wifi_stop();
        esp32_wifi_status = ESP32_WIFI_UNKNOWN;
    }
    return 0;
}

bool esp32_wifi_is_ap_enabled(void) {
    return wifi_ap_en;
}

uint8_t *esp32_wifi_ap_get_mac(void)
{
    esp_wifi_get_mac(ESP_IF_WIFI_AP, ap_mac);
    return ap_mac;
}

uint8_t *esp32_wifi_client_get_mac(void)
{
    esp_wifi_get_mac(ESP_IF_WIFI_STA, sta_mac);
    return sta_mac;
}

#if 0 //this is for REST API
static void http_get_task(void *pvParameters)
{
    const struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
    };
    struct addrinfo *res;
    struct in_addr *addr;
    int s, r;
    char recv_buf[64];
    int data_received = 0;

    while(1) {
        /* Wait for the callback to set the CONNECTED_BIT in the
           event group.
        */
        xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT,
                            false, true, portMAX_DELAY);
        ESP_LOGI(TAG, "Connected to AP");

        int err = getaddrinfo(WEB_SERVER, WEB_PORT, &hints, &res);

        if(err != 0 || res == NULL) {
            ESP_LOGE(TAG, "DNS lookup failed err=%d res=%p", err, res);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }

        /* Code to print the resolved IP.
         * Note: inet_ntoa is non-reentrant, look at ipaddr_ntoa_r for "real" code 
         */
        addr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
        ESP_LOGI(TAG, "DNS lookup succeeded. IP=%s", inet_ntoa(*addr));

        s = socket(res->ai_family, res->ai_socktype, 0);
        if(s < 0) {
            ESP_LOGE(TAG, "... Failed to allocate socket.");
            freeaddrinfo(res);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG, "... allocated socket\r\n");

        if(connect(s, res->ai_addr, res->ai_addrlen) != 0) {
            ESP_LOGE(TAG, "... socket connect failed errno=%d", errno);
            close(s);
            freeaddrinfo(res);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }

        ESP_LOGI(TAG, "... connected");
        freeaddrinfo(res);

        if (write(s, REQUEST, strlen(REQUEST)) < 0) {
            ESP_LOGE(TAG, "... socket send failed");
            close(s);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG, "... socket send success");

        /* Read HTTP response */
        do {
            bzero(recv_buf, sizeof(recv_buf));
            r = read(s, recv_buf, sizeof(recv_buf)-1);
            for(int i = 0; i < r; i++) {
                putchar(recv_buf[i]);
            }
        } while(r > 0);

        ESP_LOGI(TAG, "... done reading from socket. Last read return=%d errno=%d\r\n", r, errno);
        close(s);
        for(int countdown = 10; countdown >= 0; countdown--) {
            ESP_LOGI(TAG, "%d... ", countdown);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
        ESP_LOGI(TAG, "Starting again!");
    }
}
#endif




#ifdef P_TESTING    //    // New Added For HTTP_PS_20

#ifdef HTTP_CLIENT_Code
static const char *TAG = "HTTP_CLIENT";

esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            if (!esp_http_client_is_chunked_response(evt->client)) {
                // Write out data
                 printf("%.*s", evt->data_len, (char*)evt->data);
            }

            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_DISCONNECTED");
            break;
    }
    return ESP_OK;
}


#ifdef DATA_HTTP_BIN_ORG

 static void http_rest_with_url()
{
    esp_http_client_config_t config = {
        .url = "http://httpbin.org/get",
        .event_handler = _http_event_handler,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    // GET
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "HTTP GET Status = %d, content_length = %d",
                esp_http_client_get_status_code(client),
                esp_http_client_get_content_length(client));
    } else {
        ESP_LOGE(TAG, "HTTP GET request failed: %s", esp_err_to_name(err));
    }

    // POST
    const char *post_data = "field1=value1&field2=value2";

    esp_http_client_set_url(client, "http://httpbin.org/post");
    esp_http_client_set_method(client, HTTP_METHOD_POST);

    esp_http_client_set_post_field(client, post_data, strlen(post_data));

    err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "HTTP POST Status = %d, content_length = %d",
                esp_http_client_get_status_code(client),
                esp_http_client_get_content_length(client));
    } else {
        ESP_LOGE(TAG, "HTTP POST request failed: %s", esp_err_to_name(err));
    }

    //PUT
    esp_http_client_set_url(client, "http://httpbin.org/put");
    esp_http_client_set_method(client, HTTP_METHOD_PUT);
    err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "HTTP PUT Status = %d, content_length = %d",
                esp_http_client_get_status_code(client),
                esp_http_client_get_content_length(client));
    } else {
        ESP_LOGE(TAG, "HTTP PUT request failed: %s", esp_err_to_name(err));
    }

    //PATCH
    esp_http_client_set_url(client, "http://httpbin.org/patch");
    esp_http_client_set_method(client, HTTP_METHOD_PATCH);
    esp_http_client_set_post_field(client, NULL, 0);
    err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "HTTP PATCH Status = %d, content_length = %d",
                esp_http_client_get_status_code(client),
                esp_http_client_get_content_length(client));
    } else {
        ESP_LOGE(TAG, "HTTP PATCH request failed: %s", esp_err_to_name(err));
    }

    //DELETE
    esp_http_client_set_url(client, "http://httpbin.org/delete");
    esp_http_client_set_method(client, HTTP_METHOD_DELETE);
    err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "HTTP DELETE Status = %d, content_length = %d",
                esp_http_client_get_status_code(client),
                esp_http_client_get_content_length(client));
    } else {
        ESP_LOGE(TAG, "HTTP DELETE request failed: %s", esp_err_to_name(err));
    }

    //HEAD
    esp_http_client_set_url(client, "http://httpbin.org/get");
    esp_http_client_set_method(client, HTTP_METHOD_HEAD);
    err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "HTTP HEAD Status = %d, content_length = %d",
                esp_http_client_get_status_code(client),
                esp_http_client_get_content_length(client));
    } else {
        ESP_LOGE(TAG, "HTTP HEAD request failed: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
}

#endif


#ifdef ONLY_FOR_TESTING_DATA_SERVER

extern const char howsmyssl_com_root_cert_pem_start[] asm("_binary_howsmyssl_com_root_cert_pem_start");
extern const char howsmyssl_com_root_cert_pem_end[]   asm("_binary_howsmyssl_com_root_cert_pem_end");

static void http_rest_with_url()
{
    esp_http_client_config_t config = {
	  .url = "https://aws.amazon.com",
       .event_handler = _http_event_handler,
   };

   esp_http_client_handle_t client = esp_http_client_init(&config);
//   // GET

    // esp_http_client_set_url(client, "https://63swuc7z9c.execute-api.us-west-1.amazonaws.com/dev/apiRegistration/registration");

   // esp_http_client_set_url(client, "https://czza30pbce.execute-api.us-west-1.amazonaws.com/hellotest/hellotest");
   // esp_http_client_set_url(client, "https://9755hum0sk.execute-api.us-west-1.amazonaws.com/dev/upercase");

   // esp_http_client_set_url(client, "https://l0ui6i285c.execute-api.us-west-1.amazonaws.com/prod/createuser");  ///off
  //  esp_http_client_set_url(client, "https://63swuc7z9c.execute-api.us-west-1.amazonaws.com/dev/apiRegistration/registration?12");  // per

 //  esp_http_client_set_url(client, "https://h9kcdhbvc4.execute-api.ap-south-1.amazonaws.com/beta/get");  // per
    esp_http_client_set_url(client, " https://s268s34khg.execute-api.us-west-1.amazonaws.com/default/HeaterLambdaFunction");  // per

    esp_http_client_set_method(client, HTTP_METHOD_GET);   // New added as get was not working...
    esp_err_t err = esp_http_client_perform(client);

   if (err == ESP_OK) {
       ESP_LOGI(TAG, "HTTP GET Status = %d, content_length = %d",
               esp_http_client_get_status_code(client),
               esp_http_client_get_content_length(client));
   } else {
       ESP_LOGE(TAG, "HTTP GET request failed: %s", esp_err_to_name(err));
   }

   char post_data[60] = {0};
   // sprintf(post_data,"field1=%d&field2=%d&field3=%d", 60,70,80);

  // esp_http_client_set_url(client, "https://63swuc7z9c.execute-api.us-west-1.amazonaws.com/dev/apiRegistration/registration");   // OldOne
  // esp_http_client_set_url(client, "https://czza30pbce.execute-api.us-west-1.amazonaws.com/hellotest/hellotest");
  // esp_http_client_set_url(client, "https://9755hum0sk.execute-api.us-west-1.amazonaws.com/dev/upercase");

  // esp_http_client_set_url(client, "https://l0ui6i285c.execute-api.us-west-1.amazonaws.com/prod/createuser");  // off
  // esp_http_client_set_url(client, "https://63swuc7z9c.execute-api.us-west-1.amazonaws.com/dev/apiRegistration/registration?12");  // per
  // esp_http_client_set_url(client, " https://h9kcdhbvc4.execute-api.ap-south-1.amazonaws.com/beta/post");  // per

    esp_http_client_set_url(client, " https://s268s34khg.execute-api.us-west-1.amazonaws.com/default/HeaterLambdaFunction");  // per


   esp_http_client_set_method(client, HTTP_METHOD_POST);
   esp_http_client_set_post_field(client, post_data, strlen(post_data));

   err = esp_http_client_perform(client);
   if (err == ESP_OK) {
       ESP_LOGI(TAG, "HTTP POST Status = %d, content_length = %d",
               esp_http_client_get_status_code(client),
               esp_http_client_get_content_length(client));
   } else {
       ESP_LOGE(TAG, "HTTP POST request failed: %s", esp_err_to_name(err));
   }

    esp_http_client_cleanup(client);
}
#endif


#ifdef WORKING_AWS_HTTP_GET_POST
static void http_rest_with_url()
{
    esp_http_client_config_t config = {
	  .url = "https://aws.amazon.com",
       .event_handler = _http_event_handler,
   };

   esp_http_client_handle_t client = esp_http_client_init(&config);
//   // GET
    esp_http_client_set_url(client, "https://63swuc7z9c.execute-api.us-west-1.amazonaws.com/dev/apiRegistration/registration?12");//??

    esp_http_client_set_method(client, HTTP_METHOD_GET);   // New added as get was not working...

   esp_err_t err = esp_http_client_perform(client);

   if (err == ESP_OK) {
       ESP_LOGI(TAG, "HTTP GET Status = %d, content_length = %d",
               esp_http_client_get_status_code(client),
               esp_http_client_get_content_length(client));
   } else {
       ESP_LOGE(TAG, "HTTP GET request failed: %s", esp_err_to_name(err));
   }

   char post_data[60] = {0};
  // esp_http_client_set_url(client, "https://63swuc7z9c.execute-api.us-west-1.amazonaws.com/dev/apiRegistration/registration");
   esp_http_client_set_url(client, "https://63swuc7z9c.execute-api.us-west-1.amazonaws.com/dev/apiRegistration/registration?12");
   esp_http_client_set_method(client, HTTP_METHOD_POST);
   esp_http_client_set_post_field(client, post_data, strlen(post_data));

   err = esp_http_client_perform(client);
   if (err == ESP_OK) {
       ESP_LOGI(TAG, "HTTP POST Status = %d, content_length = %d",
               esp_http_client_get_status_code(client),
               esp_http_client_get_content_length(client));
   } else {
       ESP_LOGE(TAG, "HTTP POST request failed: %s", esp_err_to_name(err));
   }

    esp_http_client_cleanup(client);
}
#endif




#ifdef WORKING_DATA_ON_THINK_SPEAK_SERVER
// working for values on the thinkSpeak Server by HTTP_post _Begin
static void http_rest_with_url()
{
   esp_http_client_config_t config = {
       .url = "https://api.thingspeak.com",
       .event_handler = _http_event_handler,
   };
   esp_http_client_handle_t client = esp_http_client_init(&config);


//   // GET
  // esp_http_client_set_url(client, "https://api.thingspeak.com/channels/1127334/fields/1.json?api_key=0TFF9WO9TZ3A49ZH&results=2");//??

   esp_err_t err = esp_http_client_perform(client);
   if (err == ESP_OK) {
       ESP_LOGI(TAG, "HTTP GET Status = %d, content_length = %d",
               esp_http_client_get_status_code(client),
               esp_http_client_get_content_length(client));
   } else {
       ESP_LOGE(TAG, "HTTP GET request failed: %s", esp_err_to_name(err));
   }

//   // POST
  // const char *post_data = "field1=value1&field2=value2";

   // esp_err_t err;
    char post_data[10] = {0};
    sprintf(post_data,"field1=%d", 20);
   esp_http_client_set_url(client, "https://api.thingspeak.com/update?api_key=JUGQ4ANQOGJUH29P");
   esp_http_client_set_method(client, HTTP_METHOD_POST);
   esp_http_client_set_post_field(client, post_data, strlen(post_data));

   err = esp_http_client_perform(client);
   if (err == ESP_OK) {
       ESP_LOGI(TAG, "HTTP POST Status = %d, content_length = %d",
               esp_http_client_get_status_code(client),
               esp_http_client_get_content_length(client));
   } else {
       ESP_LOGE(TAG, "HTTP POST request failed: %s", esp_err_to_name(err));
   }

    esp_http_client_cleanup(client);
}
// working for values on the thinkSpeak Server by HTTP_post _End

#endif


void http_test_task(void *pvParameters)
 {
	 while(1)
	 {
 	printf("Test\n");

    // app_wifi_wait_connected();
     ESP_LOGI(TAG, "Connected to AP, begin http example");
     http_rest_with_url();
//     http_rest_with_hostname_path();
//     http_auth_basic();
//     http_auth_basic_redirect();
//     http_auth_digest();
//     http_relative_redirect();
//     http_absolute_redirect();
//     https_with_url();
//     https_with_hostname_path();
//     http_redirect_to_https();
//     http_download_chunk();
//     http_perform_as_stream_reader();
//     https_async();

     ESP_LOGI(TAG, "Finish http example");
    // vTaskDelete(NULL);
	 }// end of while
 }
#endif // ifdef HTTP_CLIENT_Code
#endif // end of // ifdef P_TESTING





#ifdef  HTTP_POST_FUNCTION_PRADEEP_SIR
#define  PROJECT_ERROR_INTERNAL 0
#define  PROJECT_SUCCESS     1
#define  DEFAULT_TIMEOUT   100    // Taken Only For testing

 uint32_t sendHttpPostJson(char * url, char * json)    //  json = data
 {
     uint32_t length = strlen(url);
     char urlCommand[50];
     uint32_t success = PROJECT_ERROR_INTERNAL;

     sprintf(urlCommand, "AT+QHTTPURL=%d,80\r\n", length);

     // Set url length
     success = sendCmdAndWaitForResp(urlCommand, "CONNECT", DEFAULT_TIMEOUT);
     if (success == PROJECT_SUCCESS)
     {
         // Set URL
         sendCmdAndWaitForResp(url, "OK", DEFAULT_TIMEOUT);
     }
     // get length of JSON
     length = strlen(json);
     ESP_LOGI(TAG,"JSON Len: %d: %s", length, json);
     //POST request length
     sprintf(urlCommand, "AT+QHTTPPOST=%d,80,80\r\n", length);
     //Someimes connect can take a long time to respond.
     success = sendCmdAndWaitForResp(urlCommand, "CONNECT", 30000);
     if (success == PROJECT_SUCCESS)
     {
         // POST JSON
         success = sendCmdAndWaitForResp(json, "+QHTTPPOST:", 2000);
     }
     return success;
 }
#endif




#ifdef thing_Shadow

 static const char *TAG = "shadow";

 #define ROOMTEMPERATURE_UPPERLIMIT 32.0f
 #define ROOMTEMPERATURE_LOWERLIMIT 25.0f
 #define STARTING_ROOMTEMPERATURE ROOMTEMPERATURE_LOWERLIMIT

  #define MAX_LENGTH_OF_UPDATE_JSON_BUFFER 200   // Original

 // #define MAX_LENGTH_OF_UPDATE_JSON_BUFFER 400   // Testing   // 6 Parameter error covered by buffer 400
 // #define MAX_LENGTH_OF_UPDATE_JSON_BUFFER 250   // Testing   // 6 Parameter error covered by buffer 400

 //// New added for testing..b
 //#define CONFIG_WIFI_SSID   "WF-HOME"
 //#define CONFIG_WIFI_PASSWORD   "bksm1554"
 //#define CONFIG_EXAMPLE_EMBEDDED_CERTS
 //#define CONFIG_AWS_EXAMPLE_THING_NAME  "Heater"
 //#define CONFIG_AWS_EXAMPLE_CLIENT_ID   "0001"
 // end


 /* The examples use simple WiFi configuration that you can set via
    'make menuconfig'.

    If you'd rather not, just change the below entries to strings with
    the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
 */
 #define EXAMPLE_WIFI_SSID CONFIG_WIFI_SSID
 #define EXAMPLE_WIFI_PASS CONFIG_WIFI_PASSWORD

 /* FreeRTOS event group to signal when we are connected & ready to make a request */
 static EventGroupHandle_t wifi_event_group;

 /* The event group allows multiple bits for each event,
    but we only care about one event - are we connected
    to the AP with an IP? */
 // const int CONNECTED_BIT = BIT0;  //Commented for testing..
 /* CA Root certificate, device ("Thing") certificate and device
  * ("Thing") key.
    Example can be configured one of two ways:
    "Embedded Certs" are loaded from files in "certs/" and embedded into the app binary.
    "Filesystem Certs" are loaded from the filesystem (SD card, etc.)
    See example README for more details.
 */
 #if defined(CONFIG_EXAMPLE_EMBEDDED_CERTS)

 extern const uint8_t aws_root_ca_pem_start[] asm("_binary_aws_root_ca_pem_start");
 extern const uint8_t aws_root_ca_pem_end[] asm("_binary_aws_root_ca_pem_end");
 extern const uint8_t certificate_pem_crt_start[] asm("_binary_certificate_pem_crt_start");
 extern const uint8_t certificate_pem_crt_end[] asm("_binary_certificate_pem_crt_end");
 extern const uint8_t private_pem_key_start[] asm("_binary_private_pem_key_start");
 extern const uint8_t private_pem_key_end[] asm("_binary_private_pem_key_end");

 #elif defined(CONFIG_EXAMPLE_FILESYSTEM_CERTS)

 static const char * DEVICE_CERTIFICATE_PATH = CONFIG_EXAMPLE_CERTIFICATE_PATH;
 static const char * DEVICE_PRIVATE_KEY_PATH = CONFIG_EXAMPLE_PRIVATE_KEY_PATH;
 static const char * ROOT_CA_PATH = CONFIG_EXAMPLE_ROOT_CA_PATH;

 #else
 #error "Invalid method for loading certs"
 #endif




 static void simulateRoomTemperature(float *pRoomTemperature) {
     static float deltaChange;

     if(*pRoomTemperature >= ROOMTEMPERATURE_UPPERLIMIT) {
         deltaChange = -0.5f;
     } else if(*pRoomTemperature <= ROOMTEMPERATURE_LOWERLIMIT) {
         deltaChange = 0.5f;
     }

     *pRoomTemperature += deltaChange;
 }

 static bool shadowUpdateInProgress;


 void ShadowUpdateStatusCallback(const char *pThingName, ShadowActions_t action, Shadow_Ack_Status_t status,
                                 const char *pReceivedJsonDocument, void *pContextData) {


     IOT_UNUSED(pThingName);
     IOT_UNUSED(action);
     IOT_UNUSED(pReceivedJsonDocument);
     IOT_UNUSED(pContextData);

     shadowUpdateInProgress = false;

     if(SHADOW_ACK_TIMEOUT == status)
     {
         ESP_LOGE(TAG, "Update timed out");
         printf("Update timed out \n ");
     } else if(SHADOW_ACK_REJECTED == status)
     {
         ESP_LOGE(TAG, "Update rejected");
         printf("Update rejected \n ");

     } else if(SHADOW_ACK_ACCEPTED == status)
     {
         ESP_LOGI(TAG, "Update accepted");
         printf("Update accepted \n ");
     }

     printf("pReceivedJsonDocument %s\n", pReceivedJsonDocument);
 }



 void windowActuate_Callback(const char *pJsonString, uint32_t JsonStringDataLen, jsonStruct_t *pContext) {
     IOT_UNUSED(pJsonString);
     IOT_UNUSED(JsonStringDataLen);

     if(pContext != NULL) {
         ESP_LOGI(TAG, "Delta - Window state changed to %d", *(bool *) (pContext->pData));
     }
 }



 void aws_iot_task(void *param) {
     IoT_Error_t rc = FAILURE;

     char JsonDocumentBuffer[MAX_LENGTH_OF_UPDATE_JSON_BUFFER];
     char JsonDocumentBuffer1[MAX_LENGTH_OF_UPDATE_JSON_BUFFER];

     size_t sizeOfJsonDocumentBuffer = sizeof(JsonDocumentBuffer) / sizeof(JsonDocumentBuffer[0]);
     float temperature = 0.0;
     float temperature1 = 0.0; // Testing only
     float temperature2 = 0.0;  // Testing only

     char HeaterState[12] = "HEATER_ON";
     char Thermostate_Status[12] = "Thermo_ON";

     bool windowOpen = false;  //Original
    // char windowOpen[200] = "welcome";

     jsonStruct_t windowActuator;
     windowActuator.cb = windowActuate_Callback;
     windowActuator.pData = &windowOpen;
      windowActuator.pKey = "windowOpen";   //Original
    // windowActuator.pKey = "welcome"; // TEST

      windowActuator.type = SHADOW_JSON_BOOL;
      windowActuator.dataLength = sizeof(bool);

    // windowActuator.type = SHADOW_JSON_STRING;  // TEST
    //  windowActuator.dataLength =200;  // string length

     jsonStruct_t temperatureHandler;
     temperatureHandler.cb = NULL;
     temperatureHandler.pKey = "temperature";
     temperatureHandler.pData = &temperature;
     temperatureHandler.type = SHADOW_JSON_FLOAT;
     temperatureHandler.dataLength = sizeof(float);


     //Testing..Begin
     jsonStruct_t temperatureHandler1;
     temperatureHandler1.cb = NULL;
     temperatureHandler1.pKey = "temperature1";
     temperatureHandler1.pData = &temperature1;
     temperatureHandler1.type = SHADOW_JSON_FLOAT;
     temperatureHandler1.dataLength = sizeof(float);
    //Test End

     //Testing..Begin
         jsonStruct_t temperatureHandler2;
         temperatureHandler2.cb = NULL;
         temperatureHandler2.pKey = "temperature2";
         temperatureHandler2.pData = &temperature2;
         temperatureHandler2.type = SHADOW_JSON_FLOAT;
         temperatureHandler2.dataLength = sizeof(float);
        //Test End

         //Testing..Begin
 		jsonStruct_t HeaterStateHandler;
 		HeaterStateHandler.cb = NULL;
 		HeaterStateHandler.pKey = "Heater State";
 		HeaterStateHandler.pData = &HeaterState;
 		HeaterStateHandler.type = SHADOW_JSON_STRING;
 		HeaterStateHandler.dataLength = 12;
 	   //Test End

 //                //Testing..Begin
 //			   jsonStruct_t ThermostateHandler;
 //			   ThermostateHandler.cb = NULL;
 //			   ThermostateHandler.pKey = "ThermoStatus";
 //			   ThermostateHandler.pData = &Thermostate_Status;
 //			   ThermostateHandler.type = SHADOW_JSON_STRING;
 //			   ThermostateHandler.dataLength = 16;
 //			  //Test End

     ESP_LOGI(TAG, "AWS IoT SDK Version %d.%d.%d-%s", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_TAG);

     // initialize the mqtt client
     AWS_IoT_Client mqttClient;

     ShadowInitParameters_t sp = ShadowInitParametersDefault;
     sp.pHost = AWS_IOT_MQTT_HOST;
     // sp.pHost = "a1s70xa3tg6svk-ats.iot.ap-south-1.amazonaws.com";
     sp.port = AWS_IOT_MQTT_PORT;

 #if defined(CONFIG_EXAMPLE_EMBEDDED_CERTS)
     sp.pClientCRT = (const char *)certificate_pem_crt_start;
     sp.pClientKey = (const char *)private_pem_key_start;
     sp.pRootCA = (const char *)aws_root_ca_pem_start;
 #elif defined(CONFIG_EXAMPLE_FILESYSTEM_CERTS)
     sp.pClientCRT = DEVICE_CERTIFICATE_PATH;
     sp.pClientKey = DEVICE_PRIVATE_KEY_PATH;
     sp.pRootCA = ROOT_CA_PATH;
 #endif
     sp.enableAutoReconnect = false;
     sp.disconnectHandler = NULL;

 #ifdef CONFIG_EXAMPLE_SDCARD_CERTS
     ESP_LOGI(TAG, "Mounting SD card...");
     sdmmc_host_t host = SDMMC_HOST_DEFAULT();
     sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
     esp_vfs_fat_sdmmc_mount_config_t mount_config = {
         .format_if_mount_failed = false,
         .max_files = 3,
     };
     sdmmc_card_t* card;
     esp_err_t ret = esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot_config, &mount_config, &card);
     if (ret != ESP_OK) {
         ESP_LOGE(TAG, "Failed to mount SD card VFAT filesystem. Error: %s", esp_err_to_name(ret));
         abort();
     }
 #endif



//     /* Wait for WiFI to show as connected */   // Commented only for testing..in main firmware as wifi has that event..
 //   xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT,
  //                       false, true, portMAX_DELAY);


     ESP_LOGI(TAG, "Shadow Init");
     rc = aws_iot_shadow_init(&mqttClient, &sp);
     if(SUCCESS != rc) {
         ESP_LOGE(TAG, "aws_iot_shadow_init returned error %d, aborting...", rc);
         abort();
     }

     ShadowConnectParameters_t scp = ShadowConnectParametersDefault;

     scp.pMyThingName = CONFIG_AWS_EXAMPLE_THING_NAME;
     scp.pMqttClientId = CONFIG_AWS_EXAMPLE_CLIENT_ID;
     scp.mqttClientIdLen = (uint16_t) strlen(CONFIG_AWS_EXAMPLE_CLIENT_ID);

     ESP_LOGI(TAG, "Shadow Connect");
     rc = aws_iot_shadow_connect(&mqttClient, &scp);
     if(SUCCESS != rc) {
         ESP_LOGE(TAG, "aws_iot_shadow_connect returned error %d, aborting...", rc);
         abort();
     }

     /*
      * Enable Auto Reconnect functionality. Minimum and Maximum time of Exponential backoff are set in aws_iot_config.h
      *  #AWS_IOT_MQTT_MIN_RECONNECT_WAIT_INTERVAL
      *  #AWS_IOT_MQTT_MAX_RECONNECT_WAIT_INTERVAL
      */
     rc = aws_iot_shadow_set_autoreconnect_status(&mqttClient, true);
     if(SUCCESS != rc) {
         ESP_LOGE(TAG, "Unable to set Auto Reconnect to true - %d, aborting...", rc);
         abort();
     }


     rc = aws_iot_shadow_register_delta(&mqttClient, &windowActuator);   // Commented for Testing
     if(SUCCESS != rc) {
         ESP_LOGE(TAG, "Shadow Register Delta Error");
     }

     temperature = STARTING_ROOMTEMPERATURE;
     temperature1 = 85.0;   // Added Fonly for Testing
     temperature2 = 95.0;   // Added Fonly for Testing

     // loop and publish a change in temperature
     while(NETWORK_ATTEMPTING_RECONNECT == rc || NETWORK_RECONNECTED == rc || SUCCESS == rc) {
         rc = aws_iot_shadow_yield(&mqttClient, 200);
         if(NETWORK_ATTEMPTING_RECONNECT == rc || shadowUpdateInProgress) {
             rc = aws_iot_shadow_yield(&mqttClient, 1000);
             // If the client is attempting to reconnect, or already waiting on a shadow update,
             // we will skip the rest of the loop.
             continue;
         }
         ESP_LOGI(TAG, "=======================================================================================");

        // ESP_LOGI(TAG, "On Device: window state %s", windowOpen ? "true" : "false");  // commentd for test

         simulateRoomTemperature(&temperature);

         rc = aws_iot_shadow_init_json_document(JsonDocumentBuffer, sizeOfJsonDocumentBuffer);
        // printf(" After aws_iot_shadow_init_json_document \n ");

         if(SUCCESS == rc) {
         	// OriginalLines
 //            rc = aws_iot_shadow_add_reported(JsonDocumentBuffer, sizeOfJsonDocumentBuffer, 2, &temperatureHandler,
 //                                             &windowActuator);
         	//printf("before Shadow Add Reported\n ");
         	//Testing only..Begin
             rc = aws_iot_shadow_add_reported(JsonDocumentBuffer, sizeOfJsonDocumentBuffer, 5, &temperatureHandler,
                                              &windowActuator, &temperatureHandler1,  &temperatureHandler2 , &HeaterStateHandler );

 //            rc = aws_iot_shadow_add_reported(JsonDocumentBuffer, sizeOfJsonDocumentBuffer, 6, &temperatureHandler,
 //                                             &windowActuator, &temperatureHandler1,  &temperatureHandler2 , &HeaterStateHandler , &ThermostateHandler );
             //Testing only..End

             if(SUCCESS == rc) {
             	// printf("after Shadow Add Reported\n ");
                 rc = aws_iot_finalize_json_document(JsonDocumentBuffer, sizeOfJsonDocumentBuffer);
                 if(SUCCESS == rc) {
                 	// printf("after success aws_iot_finalize_json_document\n ");
                     ESP_LOGI(TAG, "Update Shadow: %s", JsonDocumentBuffer);


                     // Original
                    rc = aws_iot_shadow_update(&mqttClient, CONFIG_AWS_EXAMPLE_THING_NAME, JsonDocumentBuffer,
                                               ShadowUpdateStatusCallback, NULL, 4, true);

                     shadowUpdateInProgress = true;
                 }
             }
         }
         ESP_LOGI(TAG, "*****************************************************************************************");
         ESP_LOGI(TAG, "Stack remaining for task '%s' is %d bytes", pcTaskGetTaskName(NULL), uxTaskGetStackHighWaterMark(NULL));

         vTaskDelay(1000 / portTICK_RATE_MS);   // original
         //vTaskDelay(60000 / portTICK_RATE_MS);    // Testing..
     }

     if(SUCCESS != rc) {
         ESP_LOGE(TAG, "An error occurred in the loop %d", rc);
     }

     ESP_LOGI(TAG, "Disconnecting");
     rc = aws_iot_shadow_disconnect(&mqttClient);

     if(SUCCESS != rc) {
         ESP_LOGE(TAG, "Disconnect error %d", rc);
     }

     vTaskDelete(NULL);   // comments for testing
 }

#endif



#ifdef subscribePublishTopic


 static const char *TAG = "subpub";

 /* The examples use simple WiFi configuration that you can set via
    'make menuconfig'.
    If you'd rather not, just change the below entries to strings with
    the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
 */
 #define EXAMPLE_WIFI_SSID CONFIG_WIFI_SSID
 #define EXAMPLE_WIFI_PASS CONFIG_WIFI_PASSWORD

 /* FreeRTOS event group to signal when we are connected & ready to make a request */
 static EventGroupHandle_t wifi_event_group;

 /* The event group allows multiple bits for each event,
    but we only care about one event - are we connected
    to the AP with an IP? */
// const int CONNECTED_BIT = BIT0;   // commented for merge code..

 /* CA Root certificate, device ("Thing") certificate and device
  * ("Thing") key.
    Example can be configured one of two ways:
    "Embedded Certs" are loaded from files in "certs/" and embedded into the app binary.
    "Filesystem Certs" are loaded from the filesystem (SD card, etc.)
    See example README for more details.
 */
 #if defined(CONFIG_EXAMPLE_EMBEDDED_CERTS)

 extern const uint8_t aws_root_ca_pem_start[] asm("_binary_aws_root_ca_pem_start");
 extern const uint8_t aws_root_ca_pem_end[] asm("_binary_aws_root_ca_pem_end");
 extern const uint8_t certificate_pem_crt_start[] asm("_binary_certificate_pem_crt_start");
 extern const uint8_t certificate_pem_crt_end[] asm("_binary_certificate_pem_crt_end");
 extern const uint8_t private_pem_key_start[] asm("_binary_private_pem_key_start");
 extern const uint8_t private_pem_key_end[] asm("_binary_private_pem_key_end");

 #elif defined(CONFIG_EXAMPLE_FILESYSTEM_CERTS)

 static const char * DEVICE_CERTIFICATE_PATH = CONFIG_EXAMPLE_CERTIFICATE_PATH;
 static const char * DEVICE_PRIVATE_KEY_PATH = CONFIG_EXAMPLE_PRIVATE_KEY_PATH;
 static const char * ROOT_CA_PATH = CONFIG_EXAMPLE_ROOT_CA_PATH;

 #else
 #error "Invalid method for loading certs"
 #endif

 /**
  * @brief Default MQTT HOST URL is pulled from the aws_iot_config.h
  */
 char HostAddress[255] = AWS_IOT_MQTT_HOST;

 /**
  * @brief Default MQTT port is pulled from the aws_iot_config.h
  */
 uint32_t port = AWS_IOT_MQTT_PORT;

 void iot_subscribe_callback_handler(AWS_IoT_Client *pClient, char *topicName, uint16_t topicNameLen,
                                     IoT_Publish_Message_Params *params, void *pData) {

	 unsigned char uchTopicMatch = 1;
     char * set_Temp = "set_Temp";
     char * set_Timer = "set_Timer";
     char * set_schdule = "set_schdule";

     ESP_LOGI(TAG, "Subscribe callback");
     ESP_LOGI(TAG, "%.*s\t%.*s", topicNameLen, topicName, (int) params->payloadLen, (char *)params->payload);

     printf("\n params->payload) %s \n ", (char*) params-> payload);

//    if(uchTopicMatch == strcmp(topicName,set_Temp))
//    {
//    	uchTopic_Set_temp_subscribe_status = TRUE;
//    	uchTopic_HeaterParameter_Publish_status = FALSE;
//    	printf("set_Temp Topic \n ");
//    }
//    if(uchTopicMatch == strcmp(topicName,set_Timer))
//    {
//    	printf("set_Timer");
//    }
//     if(uchTopicMatch == strcmp(topicName,set_schdule))
//    {
//    	printf("set_schdule");
//    }

 }



 void disconnectCallbackHandler(AWS_IoT_Client *pClient, void *data) {
     ESP_LOGW(TAG, "MQTT Disconnect");
     IoT_Error_t rc = FAILURE;

     if(NULL == pClient) {
         return;
     }

     if(aws_iot_is_autoreconnect_enabled(pClient)) {
         ESP_LOGI(TAG, "Auto Reconnect is enabled, Reconnecting attempt will start now");
     } else {
         ESP_LOGW(TAG, "Auto Reconnect not enabled. Starting manual reconnect...");
         rc = aws_iot_mqtt_attempt_reconnect(pClient);
         if(NETWORK_RECONNECTED == rc) {
             ESP_LOGW(TAG, "Manual Reconnect Successful");
         } else {
             ESP_LOGW(TAG, "Manual Reconnect Failed - %d", rc);
         }
     }
 }




#define HeaterParameterSendingToAWS

 void aws_iot_task(void *param) {

	 char cPayload[100];
     int32_t i = 0;

     IoT_Error_t rc = FAILURE;

     AWS_IoT_Client client;
     IoT_Client_Init_Params mqttInitParams = iotClientInitParamsDefault;
     IoT_Client_Connect_Params connectParams = iotClientConnectParamsDefault;

//     IoT_Publish_Message_Params paramsQOS0;
//     IoT_Publish_Message_Params paramsQOS1;

#ifdef HeaterParameterSendingToAWS
     IoT_Publish_Message_Params HeaterParameter;
     IoT_Publish_Message_Params Set_Temp_Parameter;
    // IoT_Publish_Message_Params HeaterParameter;


#endif

     ESP_LOGI(TAG, "AWS IoT SDK Version %d.%d.%d-%s", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_TAG);

     mqttInitParams.enableAutoReconnect = false; // We enable this later below
     mqttInitParams.pHostURL = HostAddress;
     mqttInitParams.port = port;

 #if defined(CONFIG_EXAMPLE_EMBEDDED_CERTS)
     mqttInitParams.pRootCALocation = (const char *)aws_root_ca_pem_start;
     mqttInitParams.pDeviceCertLocation = (const char *)certificate_pem_crt_start;
     mqttInitParams.pDevicePrivateKeyLocation = (const char *)private_pem_key_start;

 #elif defined(CONFIG_EXAMPLE_FILESYSTEM_CERTS)
     mqttInitParams.pRootCALocation = ROOT_CA_PATH;
     mqttInitParams.pDeviceCertLocation = DEVICE_CERTIFICATE_PATH;
     mqttInitParams.pDevicePrivateKeyLocation = DEVICE_PRIVATE_KEY_PATH;
 #endif

     mqttInitParams.mqttCommandTimeout_ms = 20000;
     mqttInitParams.tlsHandshakeTimeout_ms = 5000;
     mqttInitParams.isSSLHostnameVerify = true;
     mqttInitParams.disconnectHandler = disconnectCallbackHandler;
     mqttInitParams.disconnectHandlerData = NULL;

 #ifdef CONFIG_EXAMPLE_SDCARD_CERTS
     ESP_LOGI(TAG, "Mounting SD card...");
     sdmmc_host_t host = SDMMC_HOST_DEFAULT();
     sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
     esp_vfs_fat_sdmmc_mount_config_t mount_config = {
         .format_if_mount_failed = false,
         .max_files = 3,
     };
     sdmmc_card_t* card;
     esp_err_t ret = esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot_config, &mount_config, &card);
     if (ret != ESP_OK) {
         ESP_LOGE(TAG, "Failed to mount SD card VFAT filesystem. Error: %s", esp_err_to_name(ret));
         abort();
     }
 #endif

     rc = aws_iot_mqtt_init(&client, &mqttInitParams);
     if(SUCCESS != rc) {
         ESP_LOGE(TAG, "aws_iot_mqtt_init returned error : %d ", rc);
         abort();
     }

     /* Wait for WiFI to show as connected */   // Commented fro testing only
//     xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT,
//                         false, true, portMAX_DELAY);

     connectParams.keepAliveIntervalInSec = 10;
     connectParams.isCleanSession = true;
     connectParams.MQTTVersion = MQTT_3_1_1;

     /* Client ID is set in the menuconfig of the example */
     connectParams.pClientID = CONFIG_AWS_EXAMPLE_CLIENT_ID;
     connectParams.clientIDLen = (uint16_t) strlen(CONFIG_AWS_EXAMPLE_CLIENT_ID);
     connectParams.isWillMsgPresent = false;

     ESP_LOGI(TAG, "Connecting to AWS...");
     do {
         rc = aws_iot_mqtt_connect(&client, &connectParams);
         if(SUCCESS != rc) {
             ESP_LOGE(TAG, "Error(%d) connecting to %s:%d", rc, mqttInitParams.pHostURL, mqttInitParams.port);
             vTaskDelay(1000 / portTICK_RATE_MS);
         }
     } while(SUCCESS != rc);

     /*
      * Enable Auto Reconnect functionality. Minimum and Maximum time of Exponential backoff are set in aws_iot_config.h
      *  #AWS_IOT_MQTT_MIN_RECONNECT_WAIT_INTERVAL
      *  #AWS_IOT_MQTT_MAX_RECONNECT_WAIT_INTERVAL
      */
     rc = aws_iot_mqtt_autoreconnect_set_status(&client, true);
     if(SUCCESS != rc) {
         ESP_LOGE(TAG, "Unable to set Auto Reconnect to true - %d", rc);
         abort();
     }

#ifdef HeaterParameterSendingToAWS

     const char *TOPIC1 = "HeaterParameter";
     const int TOPIC_LEN1 = strlen(TOPIC1);

	 const char *TOPIC2 = "set_Temp";
	 const int TOPIC_LEN2 = strlen(TOPIC2);

//     if(uchTopic_Set_temp_subscribe_status == FALSE)
//      {
         ESP_LOGI(TAG, "Subscribing...");
         rc = aws_iot_mqtt_subscribe(&client, TOPIC1, TOPIC_LEN1, QOS0, iot_subscribe_callback_handler, NULL);  // TOPIC1 = "HeaterParameter";
         if(SUCCESS != rc) {
             ESP_LOGE(TAG, "Error subscribing : %d ", rc);
             abort();
         }

		ESP_LOGI(TAG, "Subscribing...");
		rc = aws_iot_mqtt_subscribe(&client, TOPIC2, TOPIC_LEN2, QOS0, iot_subscribe_callback_handler, NULL);  // TOPIC2 = "set_Temp";
		if(SUCCESS != rc) {
		  ESP_LOGE(TAG, "Error subscribing : %d ", rc);
		  abort();
		}



//		const char *TOPIC3 = "set_Timer";
//		const int TOPIC_LEN3 = strlen(TOPIC3);
//
//		ESP_LOGI(TAG, "Subscribing...");
//		rc = aws_iot_mqtt_subscribe(&client, TOPIC3, TOPIC_LEN3, QOS0, iot_subscribe_callback_handler, NULL);
//		if(SUCCESS != rc) {
//		   ESP_LOGE(TAG, "Error subscribing : %d ", rc);
//		   abort();
//		}
//
//		const char *TOPIC4 = "set_schdule";
//		const int TOPIC_LEN4 = strlen(TOPIC4);
//
//		ESP_LOGI(TAG, "Subscribing...");
//		rc = aws_iot_mqtt_subscribe(&client, TOPIC4, TOPIC_LEN4, QOS0, iot_subscribe_callback_handler, NULL);
//		if(SUCCESS != rc) {
//			ESP_LOGE(TAG, "Error subscribing : %d ", rc);
//			abort();
//		}
#endif  // end of #ifdef HeaterParameterSendingToAWS

//     const char *TOPIC = "test_topic/esp32";
//     const int TOPIC_LEN = strlen(TOPIC);
//
//     ESP_LOGI(TAG, "Subscribing...");
//     rc = aws_iot_mqtt_subscribe(&client, TOPIC, TOPIC_LEN, QOS0, iot_subscribe_callback_handler, NULL);
//     if(SUCCESS != rc) {
//         ESP_LOGE(TAG, "Error subscribing : %d ", rc);
//         abort();
//     }

     //Commented For Testing..
//     sprintf(cPayload, "%s : %d ", "hello from SDK", i);
//     paramsQOS0.qos = QOS0;
//     paramsQOS0.payload = (void *) cPayload;
//     paramsQOS0.isRetained = 0;
//
//     paramsQOS1.qos = QOS1;
//     paramsQOS1.payload = (void *) cPayload;
//     paramsQOS1.isRetained = 0;

#ifdef HeaterParameterSendingToAWS

     char cPayload1[100];
     HeaterParameter.qos = QOS1;
     HeaterParameter.payload = (void *) cPayload1;
     HeaterParameter.isRetained = 0;

	 char cPayload2[30];
	 Set_Temp_Parameter.qos = QOS1;
	 Set_Temp_Parameter.payload = (void *) cPayload2;
	 Set_Temp_Parameter.isRetained = 0;
#endif

     while((NETWORK_ATTEMPTING_RECONNECT == rc || NETWORK_RECONNECTED == rc || SUCCESS == rc)) {

         //Max time the yield function will wait for read messages
         rc = aws_iot_mqtt_yield(&client, 100);
         if(NETWORK_ATTEMPTING_RECONNECT == rc) {
             // If the client is attempting to reconnect we will skip the rest of the loop.
             continue;
         }

         ESP_LOGI(TAG, "Stack remaining for task '%s' is %d bytes", pcTaskGetTaskName(NULL), uxTaskGetStackHighWaterMark(NULL));

     //  vTaskDelay(1000 / portTICK_RATE_MS);   //Original Lines
         vTaskDelay(1000 / portTICK_RATE_MS);

         // Commented for testing
//         sprintf(cPayload, "%s : %d ", "hello from ESP32 (QOS0)", i++);
//         paramsQOS0.payloadLen = strlen(cPayload);
//         rc = aws_iot_mqtt_publish(&client, TOPIC, TOPIC_LEN, &paramsQOS0);
//
//         sprintf(cPayload, "%s : %d ", "hello from ESP32 (QOS1)", i++);
//         paramsQOS1.payloadLen = strlen(cPayload);
//         rc = aws_iot_mqtt_publish(&client, TOPIC, TOPIC_LEN, &paramsQOS1);


#ifdef HeaterParameterSendingToAWS

//        if(uchTopic_Set_temp_subscribe_status == TRUE)
//         {
//        	printf("\n I am in Set Temp paramter publish \n\n ");
//        	memset(cPayload2,0,sizeof(cPayload2));
//        	sprintf(cPayload2, "%s : %d", "Temperature is set to ",40);
//        	Set_Temp_Parameter.payloadLen = strlen(cPayload2);
//            rc = aws_iot_mqtt_publish(&client, TOPIC2, TOPIC_LEN2, &Set_Temp_Parameter);
//            uchTopic_Set_temp_subscribe_status = FALSE;
//            uchTopic_HeaterParameter_Publish_status = TRUE;
//           // data->manual_temperature_celsius = 40;
//         }

//        if(uchTopic_HeaterParameter_Publish_status == TRUE)
//          {
        	   printf("\n I am in Topic_HeaterParameter_Publish\n ");
				// uchTopic_Set_temp_subscribe_status = TRUE;
				memset(cPayload1,0,sizeof(cPayload1));
				sprintf(cPayload1, "%s : %d  %s : %d %s : %d %s : %d %s : %d", "Ambient Temp", 40,"Set Temp", 50, "Heater Status", 1, "Timer",1, "Schedule", 0);
				HeaterParameter.payloadLen = strlen(cPayload1);
				rc = aws_iot_mqtt_publish(&client, TOPIC1, TOPIC_LEN1, &HeaterParameter);
        //  }
#endif

         //  printf("After publish HeaterParameterSendingToAWS\n ");
         if (rc == MQTT_REQUEST_TIMEOUT_ERROR) {
             ESP_LOGW(TAG, "QOS1 publish ack not received.");
             rc = SUCCESS;
         }
     }

     ESP_LOGE(TAG, "An error occurred in the main loop.");
    // abort();  // Commented Abort for Tesing
 }



//  Working AWS_IOt_task
// void aws_iot_task(void *param) {
//     char cPayload[100];
//
//     int32_t i = 0;
//
//     IoT_Error_t rc = FAILURE;
//
//     AWS_IoT_Client client;
//     IoT_Client_Init_Params mqttInitParams = iotClientInitParamsDefault;
//     IoT_Client_Connect_Params connectParams = iotClientConnectParamsDefault;
//
//     IoT_Publish_Message_Params paramsQOS0;
//     IoT_Publish_Message_Params paramsQOS1;
//
//     ESP_LOGI(TAG, "AWS IoT SDK Version %d.%d.%d-%s", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_TAG);
//
//     mqttInitParams.enableAutoReconnect = false; // We enable this later below
//     mqttInitParams.pHostURL = HostAddress;
//     mqttInitParams.port = port;
//
// #if defined(CONFIG_EXAMPLE_EMBEDDED_CERTS)
//     mqttInitParams.pRootCALocation = (const char *)aws_root_ca_pem_start;
//     mqttInitParams.pDeviceCertLocation = (const char *)certificate_pem_crt_start;
//     mqttInitParams.pDevicePrivateKeyLocation = (const char *)private_pem_key_start;
//
// #elif defined(CONFIG_EXAMPLE_FILESYSTEM_CERTS)
//     mqttInitParams.pRootCALocation = ROOT_CA_PATH;
//     mqttInitParams.pDeviceCertLocation = DEVICE_CERTIFICATE_PATH;
//     mqttInitParams.pDevicePrivateKeyLocation = DEVICE_PRIVATE_KEY_PATH;
// #endif
//
//     mqttInitParams.mqttCommandTimeout_ms = 20000;
//     mqttInitParams.tlsHandshakeTimeout_ms = 5000;
//     mqttInitParams.isSSLHostnameVerify = true;
//     mqttInitParams.disconnectHandler = disconnectCallbackHandler;
//     mqttInitParams.disconnectHandlerData = NULL;
//
// #ifdef CONFIG_EXAMPLE_SDCARD_CERTS
//     ESP_LOGI(TAG, "Mounting SD card...");
//     sdmmc_host_t host = SDMMC_HOST_DEFAULT();
//     sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
//     esp_vfs_fat_sdmmc_mount_config_t mount_config = {
//         .format_if_mount_failed = false,
//         .max_files = 3,
//     };
//     sdmmc_card_t* card;
//     esp_err_t ret = esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot_config, &mount_config, &card);
//     if (ret != ESP_OK) {
//         ESP_LOGE(TAG, "Failed to mount SD card VFAT filesystem. Error: %s", esp_err_to_name(ret));
//         abort();
//     }
// #endif
//
//     rc = aws_iot_mqtt_init(&client, &mqttInitParams);
//     if(SUCCESS != rc) {
//         ESP_LOGE(TAG, "aws_iot_mqtt_init returned error : %d ", rc);
//         abort();
//     }
//
//     /* Wait for WiFI to show as connected */
//     xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT,
//                         false, true, portMAX_DELAY);
//
//     connectParams.keepAliveIntervalInSec = 10;
//     connectParams.isCleanSession = true;
//     connectParams.MQTTVersion = MQTT_3_1_1;
//
//     /* Client ID is set in the menuconfig of the example */
//     connectParams.pClientID = CONFIG_AWS_EXAMPLE_CLIENT_ID;
//     connectParams.clientIDLen = (uint16_t) strlen(CONFIG_AWS_EXAMPLE_CLIENT_ID);
//     connectParams.isWillMsgPresent = false;
//
//     ESP_LOGI(TAG, "Connecting to AWS...");
//     do {
//         rc = aws_iot_mqtt_connect(&client, &connectParams);
//         if(SUCCESS != rc) {
//             ESP_LOGE(TAG, "Error(%d) connecting to %s:%d", rc, mqttInitParams.pHostURL, mqttInitParams.port);
//             vTaskDelay(1000 / portTICK_RATE_MS);
//         }
//     } while(SUCCESS != rc);
//
//     /*
//      * Enable Auto Reconnect functionality. Minimum and Maximum time of Exponential backoff are set in aws_iot_config.h
//      *  #AWS_IOT_MQTT_MIN_RECONNECT_WAIT_INTERVAL
//      *  #AWS_IOT_MQTT_MAX_RECONNECT_WAIT_INTERVAL
//      */
//     rc = aws_iot_mqtt_autoreconnect_set_status(&client, true);
//     if(SUCCESS != rc) {
//         ESP_LOGE(TAG, "Unable to set Auto Reconnect to true - %d", rc);
//         abort();
//     }
//
//     const char *TOPIC = "test_topic/esp32";
//     const int TOPIC_LEN = strlen(TOPIC);
//
//     ESP_LOGI(TAG, "Subscribing...");
//     rc = aws_iot_mqtt_subscribe(&client, TOPIC, TOPIC_LEN, QOS0, iot_subscribe_callback_handler, NULL);
//     if(SUCCESS != rc) {
//         ESP_LOGE(TAG, "Error subscribing : %d ", rc);
//         abort();
//     }
//
//     sprintf(cPayload, "%s : %d ", "hello from SDK", i);
//
//     paramsQOS0.qos = QOS0;
//     paramsQOS0.payload = (void *) cPayload;
//     paramsQOS0.isRetained = 0;
//
//     paramsQOS1.qos = QOS1;
//     paramsQOS1.payload = (void *) cPayload;
//     paramsQOS1.isRetained = 0;
//
//     while((NETWORK_ATTEMPTING_RECONNECT == rc || NETWORK_RECONNECTED == rc || SUCCESS == rc)) {
//
//         //Max time the yield function will wait for read messages
//         rc = aws_iot_mqtt_yield(&client, 100);
//         if(NETWORK_ATTEMPTING_RECONNECT == rc) {
//             // If the client is attempting to reconnect we will skip the rest of the loop.
//             continue;
//         }
//
//         ESP_LOGI(TAG, "Stack remaining for task '%s' is %d bytes", pcTaskGetTaskName(NULL), uxTaskGetStackHighWaterMark(NULL));
//         vTaskDelay(1000 / portTICK_RATE_MS);
//         sprintf(cPayload, "%s : %d ", "hello from ESP32 (QOS0)", i++);
//         paramsQOS0.payloadLen = strlen(cPayload);
//         rc = aws_iot_mqtt_publish(&client, TOPIC, TOPIC_LEN, &paramsQOS0);
//
//         sprintf(cPayload, "%s : %d ", "hello from ESP32 (QOS1)", i++);
//         paramsQOS1.payloadLen = strlen(cPayload);
//         rc = aws_iot_mqtt_publish(&client, TOPIC, TOPIC_LEN, &paramsQOS1);
//         if (rc == MQTT_REQUEST_TIMEOUT_ERROR) {
//             ESP_LOGW(TAG, "QOS1 publish ack not received.");
//             rc = SUCCESS;
//         }
//     }
//
//     ESP_LOGE(TAG, "An error occurred in the main loop.");
//     abort();
// }


#define Wifi_sub_pub

#ifdef Wifi_sub_pub
   void initialise_wifi(void);
   void disconnectCallbackHandler(AWS_IoT_Client *pClient, void *data);
 //  static esp_err_t event_handler(void *ctx, system_event_t *event);
#endif


#ifdef Wifi_sub_pub
 // static esp_err_t event_handler(void *ctx, system_event_t *event)
  esp_err_t event_handler(void *ctx, system_event_t *event)
 {
     switch(event->event_id) {
     case SYSTEM_EVENT_STA_START:
         esp_wifi_connect();
         break;
     case SYSTEM_EVENT_STA_GOT_IP:
         xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
         break;
     case SYSTEM_EVENT_STA_DISCONNECTED:
         /* This is a workaround as ESP32 WiFi libs don't currently
            auto-reassociate. */
         esp_wifi_connect();
         xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
         break;
     default:
         break;
     }
     return ESP_OK;
 }


// void disconnectCallbackHandler(AWS_IoT_Client *pClient, void *data) {
//     ESP_LOGW(TAG, "MQTT Disconnect");
//     IoT_Error_t rc = FAILURE;
//
//     if(NULL == pClient) {
//         return;
//     }
//
//     if(aws_iot_is_autoreconnect_enabled(pClient)) {
//         ESP_LOGI(TAG, "Auto Reconnect is enabled, Reconnecting attempt will start now");
//     } else {
//         ESP_LOGW(TAG, "Auto Reconnect not enabled. Starting manual reconnect...");
//         rc = aws_iot_mqtt_attempt_reconnect(pClient);
//         if(NETWORK_RECONNECTED == rc) {
//             ESP_LOGW(TAG, "Manual Reconnect Successful");
//         } else {
//             ESP_LOGW(TAG, "Manual Reconnect Failed - %d", rc);
//         }
//     }
// }


void initialise_wifi(void)
 {
     tcpip_adapter_init();
     wifi_event_group = xEventGroupCreate();
     ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
     wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
     ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
     ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );

     wifi_config_t wifi_config = {
         .sta = {
             .ssid = EXAMPLE_WIFI_SSID,
             .password = EXAMPLE_WIFI_PASS,
         },
     };

 //    wifi_config_t wifi_config = {
 //        .sta = {
 //            .ssid = "WF-Home",
 //            .password = "bksm1554",
 //        },
 //    };

     ESP_LOGI(TAG, "Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
     ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
     ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
     ESP_ERROR_CHECK( esp_wifi_start() );
 }
#endif//  end of #ifdef Wifi_sub_pub


#endif  // end for subscribe and publish..






