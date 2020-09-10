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


#ifdef P_TESTING
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


int event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
        //deprecated
        case SYSTEM_EVENT_WIFI_READY:
        case SYSTEM_EVENT_SCAN_DONE:
            break;
            //deprecated

            //WIFI_MODE_STA
        case SYSTEM_EVENT_STA_START:
            esp_wifi_connect();
            break;
        case SYSTEM_EVENT_STA_STOP:
            break;
        case SYSTEM_EVENT_STA_CONNECTED:
            if (wifi_conn_stat_callback)
                wifi_conn_stat_callback(1);
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            /* This is a workaround as ESP32 WiFi libs don't currently
               auto-reassociate. */
            printf("disconnected\n");
            esp_wifi_connect();
            xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
            if (wifi_conn_stat_callback)
                wifi_conn_stat_callback(0);
            break;
        case SYSTEM_EVENT_STA_AUTHMODE_CHANGE:
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
            printf("connected\n");
            break;
            //case SYSTEM_EVENT_AP_STA_GOT_IP:  //no macro like this TODO
            //break;

            //WIFI_MODE_AP
        case SYSTEM_EVENT_AP_START:
            wifi_ap_en = true;
            break;
        case SYSTEM_EVENT_AP_STOP:
            wifi_ap_en = false;
            break;
        case SYSTEM_EVENT_AP_STACONNECTED:
            if (wifi_conn_stat_callback)
                wifi_conn_stat_callback(1);
            break;
        case SYSTEM_EVENT_AP_STADISCONNECTED:
            if (wifi_conn_stat_callback)
                wifi_conn_stat_callback(0);
            break;
        case SYSTEM_EVENT_AP_PROBEREQRECVED:
            break;

            //WIFI WPS
        case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:
            esp_wifi_wps_disable();
            // save SSID and password
            memset(&global_wifi_config, 0, sizeof(global_wifi_config));
            esp_wifi_get_config(WIFI_IF_STA, &global_wifi_config);
            set_string_to_storage(NVS_LUCIDTRON_SSID_KEY, (char *)(global_wifi_config.sta.ssid));
            set_string_to_storage(NVS_LUCIDTRON_PW_KEY, (char *)(global_wifi_config.sta.password));
            esp_wifi_connect();
            break;
        case SYSTEM_EVENT_STA_WPS_ER_FAILED:
            break;
        case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:
            break;
        case SYSTEM_EVENT_STA_WPS_ER_PIN:
            break;

        default:
            break;
    }
    return ESP_OK;
}

int esp32_initialise_wifi(void)
{
    //this init the high level protocol handler for the driver
    tcpip_adapter_init();
    wifi_event_group = xEventGroupCreate();

    //prepare the event callback
    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );

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

    esp_http_client_set_url(client, "https://l0ui6i285c.execute-api.us-west-1.amazonaws.com/prod/createuser");
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

   esp_http_client_set_url(client, "https://l0ui6i285c.execute-api.us-west-1.amazonaws.com/prod/createuser");
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

