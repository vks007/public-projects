#ifndef CONFIG_H
#define CONFIG_H

#define TOUCH_SENSOR1 1
// define more devices here as 2, 3, ... and provide config values for each device below

#if (DEVICE == TOUCH_SENSOR1)
  #define SERIAL_DEBUG            IN_USE 
  #define SECURITY                NOT_IN_USE // using security or not to encrypt messages
  #define MY_ROLE                 ESP_NOW_ROLE_IDLE  // This is reduntant for ESP32 and only applicable for ESP8266
  #define DEVICE_NAME             "touch_sensor1" //max 15 characters without spaces
  uint8_t gatewayAddress[] =      GATEWAY_FF_AP_MAC; //comes from secrets.h
  constexpr char WIFI_SSID[] =    primary_ssid;// from secrets.h , SSID to which the Slave connects for WiFi, only SSID name needed here, not password
  #define THRESHOLD               40 // Threshold for touch pin sensitivity , greater the value, more the sensitivity
  #define TOUCHPIN0               T0 // GPIO4
  #define TOUCHPIN1               T1 // GPIO0
  #define TOUCHPIN2               T2 // GPIO2
  #define TOUCHPIN3               T3 // GPIO15
  #define TOUCHPIN4               T4 // GPIO13
  #define TOUCHPIN5               T5 // GPIO12 // this works with ESP32 CAM , other touch pins are not usable on it
  #define TOUCHPIN6               T6 // GPIO14 // this works with ESP32 CAM , other touch pins are not usable on it
  #define TOUCHPIN7               T7 // GPIO27
  #define TOUCHPIN8               T8 // GPIO33
  #define TOUCHPIN9               T9 // GPIO32
#else
  #error "Device type not found. Have you passed DEVICE id in platform.ini as build flag. See Config.h for all DEVICES"
#endif

#if USING(SECURITY)
uint8_t kok[16]= PMK_KEY_STR;//comes from secrets.h
uint8_t key[16] = LMK_KEY_STR;// comes from secrets.h
#endif


#endif
