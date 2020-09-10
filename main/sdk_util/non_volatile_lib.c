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

#include <ctype.h>
#include "nvs_flash.h"

#include "non_volatile_lib.h"      // Original Line
// #include "./sdk_util/include/non_volatile_lib.h"   // Testing line in the code suggested by Pradeep sir


#include "common.h"
//TODO: #include "debug.h"

#include "esp_spiffs.h"

nvs_handle lucidtron_nvs_handle;

int nvs_storage_init(void)
{
    int err;



/*
    nvs_flash_init();

    err = nvs_open(NVS_LUCIDTRON_NAMESPACE, NVS_READWRITE,
        &lucidtron_nvs_handle);
    if(err != ESP_OK)
    {
        //LOG_EMERG("NVS init failed\n");
        return ERROR;
    }
    else
        return SUCCESS;
*/

    esp_vfs_spiffs_conf_t conf = {
      .base_path = "/spiffs",
      .partition_label = NULL,
      .max_files = 5,
      .format_if_mount_failed = true
    };
    err = esp_vfs_spiffs_register(&conf);
    if (err != ESP_OK) {
        if (err == ESP_FAIL) {
            printf("Failed to mount or format filesystem\r\n");
        } else if (err == ESP_ERR_NOT_FOUND) {
            printf("Failed to find SPIFFS partition\r\n");
        } else {
            printf("Failed to initialize SPIFFS (%d)\r\n", err);
        }
    }
    size_t total = 0, used = 0;
    err = esp_spiffs_info(NULL, &total, &used);
    if (err != ESP_OK) {
        printf("Failed to get SPIFFS partition information\r\n");
    } else {
        printf("Partition size: total: %d, used: %d\r\n", total, used);
    }

    err |= nvs_flash_init();
    if (err != ESP_OK)    // Original Lines
    {
    	printf("\nError in nvs_flash_init():\n");
    	return ERROR;     // Original Lines
    }

    // Only For Testing _Begin..//
//    if (err != ESP_OK)
//    {
//    	  printf("not Init NVS\n "); // Added_PS2020
//          ESP_ERROR_CHECK(nvs_flash_erase());  // Added_PS2020
//          err |= nvs_flash_init();
//          if (err != ESP_OK)
//          {    printf("not Init NVS after erase ...\n "); // Added_PS2020
//        	  return ERROR;   // commentd only for testingAdded_PS2020
//          }
//    }

    printf("Init NVS\n "); // Added_PS2020
    // End//


    return SUCCESS;
}

int erase_storage_all(void)
{
    int err;

    err = nvs_open(NVS_LUCIDTRON_NAMESPACE, NVS_READWRITE,
        &lucidtron_nvs_handle);
    err |= nvs_erase_all(lucidtron_nvs_handle);
    err |= nvs_commit(lucidtron_nvs_handle);
    nvs_close(lucidtron_nvs_handle);

    if (err != ESP_OK)
        return ERROR;

    return SUCCESS;
}

int erase_string_in_storage(char* key)
{
    int err;

    err = nvs_open(NVS_LUCIDTRON_NAMESPACE, NVS_READWRITE,
        &lucidtron_nvs_handle);
    err |= nvs_erase_key(lucidtron_nvs_handle, key);
    err |= nvs_commit(lucidtron_nvs_handle);
    nvs_close(lucidtron_nvs_handle);

    if (err != ESP_OK)
        return ERROR;

    return SUCCESS;
}
int set_string_to_storage(char* key, char* value)
{
    int err;

    err = nvs_open(NVS_LUCIDTRON_NAMESPACE, NVS_READWRITE,
        &lucidtron_nvs_handle);
    err |= nvs_set_str(lucidtron_nvs_handle, key, value);
    err |= nvs_commit(lucidtron_nvs_handle);
    nvs_close(lucidtron_nvs_handle);

    if (err != ESP_OK)
        return ERROR;

    return SUCCESS;
}
int get_string_from_storage(char* key, char* buffer)
{
    size_t size;
    int i;
    int err;

    err = nvs_open(NVS_LUCIDTRON_NAMESPACE, NVS_READWRITE,
        &lucidtron_nvs_handle);
    err |= nvs_get_str(lucidtron_nvs_handle, key, NULL, &size);
    err |= nvs_get_str(lucidtron_nvs_handle, key, buffer, &size);
    nvs_close(lucidtron_nvs_handle);

    for(i = 0; i<size; i++)
    {
        if(isprint((int) buffer[i]) == 0)
        {
            buffer[i] = 0;
            printf("garbage nvs detected\n");
            return ERROR;
        }
    }

    if (err != ESP_OK)
        return ERROR;

    return SUCCESS;
}

int erase_integer_in_storage(char* key)
{
    int err;

    err = nvs_open(NVS_LUCIDTRON_NAMESPACE, NVS_READWRITE,
        &lucidtron_nvs_handle);
    err |= nvs_erase_key(lucidtron_nvs_handle, key);
    err |= nvs_commit(lucidtron_nvs_handle);
    nvs_close(lucidtron_nvs_handle);

    if (err != ESP_OK)
        return ERROR;

    return SUCCESS;
}
int set_integer_to_storage(char* key, int value)
{
    int err;

    err = nvs_open(NVS_LUCIDTRON_NAMESPACE, NVS_READWRITE,
        &lucidtron_nvs_handle);
    err |= nvs_set_i64(lucidtron_nvs_handle, key, (int64_t) value);
    err |= nvs_commit(lucidtron_nvs_handle);
    nvs_close(lucidtron_nvs_handle);

    if (err != ESP_OK)
        return ERROR;

    return SUCCESS;
}
int get_integer_from_storage(char* key, int* value)
{
    int err;

    err = nvs_open(NVS_LUCIDTRON_NAMESPACE, NVS_READWRITE,
        &lucidtron_nvs_handle);
    err |= nvs_get_i64(lucidtron_nvs_handle, key,(int64_t*) value);
    nvs_close(lucidtron_nvs_handle);

    if (err != ESP_OK)
        return ERROR;

    return SUCCESS;
}

int erase_data_in_storage(char* key)
{
    int err;

    err = nvs_open(NVS_LUCIDTRON_NAMESPACE, NVS_READWRITE,
        &lucidtron_nvs_handle);
    err |= nvs_erase_key(lucidtron_nvs_handle, key);
    err |= nvs_commit(lucidtron_nvs_handle);
    nvs_close(lucidtron_nvs_handle);

    if (err != ESP_OK)
        return ERROR;

    return SUCCESS;
}
int set_data_to_storage(char* key, void *data, size_t len)
{
    int err;

    err = nvs_open(NVS_LUCIDTRON_NAMESPACE, NVS_READWRITE,
        &lucidtron_nvs_handle);
    err |= nvs_set_blob(lucidtron_nvs_handle, key, data, len);
    err |= nvs_commit(lucidtron_nvs_handle);
    nvs_close(lucidtron_nvs_handle);

    if (err != ESP_OK)
        return ERROR;

    return SUCCESS;
}
int get_data_from_storage(char* key, void *data)
{
    size_t size;
    int err;

    err = nvs_open(NVS_LUCIDTRON_NAMESPACE, NVS_READWRITE,
        &lucidtron_nvs_handle);
    err |= nvs_get_blob(lucidtron_nvs_handle, key, NULL, &size);
    err |= nvs_get_blob(lucidtron_nvs_handle, key, data, &size);
    nvs_close(lucidtron_nvs_handle);

    if (err != ESP_OK)
        return ERROR;

    return SUCCESS;
}

