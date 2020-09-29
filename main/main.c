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

/*! \mainpage Lucidtron Longhorn Firmware
 *  \section sub1 Longhorn Source Code
 *
 *  \subsection sub2 Configuring the Project
 *
 *  make menuconfig
 *
 *  Opens a text-based configuration menu for the project.\n\n
 *  Use up & down arrow keys to navigate the menu.\n
 *  Use Enter key to go into a submenu, Escape key to go out or to exit.\n
 *  Type ? to see a help screen. Enter key exits the help screen.\n
 *  Use Space key, or Y and N keys to enable (Yes) and disable (No) 
 *      configuration items with checkboxes "`[*]`"\n
 *  Pressing ? while highlighting a configuration item displays help about 
 *      that item.\n
 *  Type / to search the configuration items.\n\n
 *
 *  Once done configuring, press Escape multiple times to exit and say 
 *      "Yes" to save the new configuration when prompted.
 *
 *  \subsection sub3 Compiling the Project
 *
 *  make all - compiles the app and bootloader and generates a partition 
 *      table based on the configuration\n
 *  make bootloader - compiles the bootloader only\n
 *  make app - builds the app only\n
 *
 *  \subsection sub4 Flashing the Project
 *
 *  make flash - updates both the application and bootloader\n
 *  make bootloader-flash - updates the bootloader only\n
 *  make app-flash - updates the application only\n 
 *
 *  \subsection sub5 Erasing Flash
 *
 *  make erase_flash
 *
 *  \subsection sub6 Viewing the Serial Output
 *
 *  make monitor
 *
 * \file app.h
 * \file button.h
 * \file communication_msg_handler.h
 * \file communication_server.h
 * \file communication_server_hal.h
 * \file display.h
 * \file display_icon.h
 * \file heater.h
 * \file led.h
 * \file lightsensor.h
 * \file lucidtron_core.h
 * \file ntp.h
 * \file tempsensor.h
 * \file version.h
 * \file wifi/include/wifi_core.h
 * \file common/include/common.h
 * \file sdk_util/include/non_volatile_lib.h
 * \file sdk_util/include/sdk_util.h
 * \file sdk_util/include/ota_update.h
 */

#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "driver/i2c.h"
#include "sdkconfig.h"

#include "display.h"
#include "button.h"

#include "led.c"   // Original
// #include "led.h"   // Testing only

#include "tempsensor.h"
#include "lightsensor.h"
#include "heater.h"

#include "non_volatile_lib.h"           // Original line in the code
// #include "./sdk_util/include/non_volatile_lib.h"   // Testing line in the code suggested by Pradeep sir

#include "app.h"


/* 
 *  Starting point of ESP32
 */ 
void app_main(void)
{
    /* Hardware peripherals init start */
    led_init();
    nvs_storage_init();  // Commneted for testing
    display_init();
    button_init();
    tempsensor_init();
    lightsensor_init();
    heater_init();
    /* Hardware peripherals init end */

    /* app proper */
    app_init();
}

