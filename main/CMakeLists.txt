set(COMPONENT_SRCS "main.c" "app.c" "led.c" "heater.c" "button.c"  "display.c"  "lightsensor.c"  "ntp.c" "tempsensor.c" "version.c" 
                   "display_icon.c" "C:/E_HEATER/longhorn_fw_v1.6.0/longhorn_fw_v1.6.0/components/display/sh1106/sh1106.c"
                   "C:/E_HEATER/longhorn_fw_v1.6.0/longhorn_fw_v1.6.0/components/clock/clock.c" 
                   "communication_msg_handler.c" 
                   "communication_server_hal.c" "communication_server.c" 
                   "./common/common.c"
                   "./wifi/wifi_core.c"
                   "./common/filesystem_lib.c"
                   "./sdk_util/sdk_util.c"
                   "./sdk_util/ota_update.c"
                   "./sdk_util/non_volatile_lib.c"
                   "C:/E_HEATER/longhorn_fw_v1.6.0/longhorn_fw_v1.6.0/components/mqtt_client/mqtt.c" 
                   "C:/E_HEATER/longhorn_fw_v1.6.0/longhorn_fw_v1.6.0/components/display/font/dejavu_sans.c"
                   "./thing_Shadow/thing_shadow_sample.c")
                   
set(COMPONENT_ADD_INCLUDEDIRS "." "C:/E_HEATER/longhorn_fw_v1.6.0/longhorn_fw_v1.6.0/components/clock/include"
                     "C:/E_HEATER/longhorn_fw_v1.6.0/longhorn_fw_v1.6.0/components/display/include" 
                     "./sdk_util/include"
                     "./common/include"
                     "./wifi/include"
                     "C:/E_HEATER/longhorn_fw_v1.6.0/longhorn_fw_v1.6.0/components/mqtt_client/include"
                     "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/esp32/include"
                     "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/soc/esp32/include/soc"
                     "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/esp32/include"
                     "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/esp_adc_cal/include"
                     "C:/E_HEATER/longhorn_fw_v1.6.0/longhorn_fw_v1.6.0/sdk_list/esp-idf_40/components/nvs_flash/include"
                     "C:/E_HEATER/longhorn_fw_v1.6.0/longhorn_fw_v1.6.0/sdk_list/esp-idf_40/components/freertos/include"
                     "C:/E_HEATER/longhorn_fw_v1.6.0/longhorn_fw_v1.6.0/sdk_list/esp-idf_40/components/esp_rom/include"
                     "C:/E_HEATER/longhorn_fw_v1.6.0/longhorn_fw_v1.6.0/sdk_list/esp-idf_40/components/esp_common/include" 
                     "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/aws_iot/include"
                     "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/aws_iot/aws-iot-device-sdk-embedded-C/include")

register_component()


if(CONFIG_EXAMPLE_EMBEDDED_CERTS)
target_add_binary_data(${COMPONENT_TARGET} "certs/aws-root-ca.pem" TEXT)
target_add_binary_data(${COMPONENT_TARGET} "certs/certificate.pem.crt" TEXT)
target_add_binary_data(${COMPONENT_TARGET} "certs/private.pem.key" TEXT)
endif()


