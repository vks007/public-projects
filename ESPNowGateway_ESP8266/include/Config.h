
#ifndef GATEWAY_CONFIG_H
#define GATEWAY_CONFIG_H

// Define all your devices here and then pass the DEVICE in the build flags in platform.ini file
#define GATEWAY_GF 1
#define GATEWAY_FF 2

#if (DEVICE == GATEWAY_GF)
  #define DEVICE_NAME             "gateway_gf" //no spaces as this is used in topic names too
  #define MQTT_TOPIC              "home/espnow/" DEVICE_NAME
  #define MQTT_BASE_TOPIC          "home/espnow"
  #define ESP_IP_ADDRESS          IP_gateway_gf // from secrets.h
  #define WiFi_SSID               gf_ssid //from secrets.h
  #define WiFi_SSID_PSWD          gf_ssid_pswd //from secrets.h
  #define STATUS_LED              2
#elif (DEVICE == GATEWAY_FF)
  #define DEVICE_NAME             "gateway_ff" //no spaces as this is used in topic names too
  #define MQTT_TOPIC              "home/espnow/" DEVICE_NAME
  #define MQTT_BASE_TOPIC          "home/espnow"
  #define ESP_IP_ADDRESS          IP_gateway_ff //from secrets.h
  #define WiFi_SSID               primary_ssid //from secrets.h
  #define WiFi_SSID_PSWD          primary_ssid_pswd //from secrets.h
  #define STATUS_LED              2 //GPIO on which the status led is connected
#else
  #error "Device type not found. Have you passed DEVICE id in platform.ini as build flag. See Config.h for all DEVICES"
#endif

#endif
