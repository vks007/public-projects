#ifndef CONFIG_H
#define CONFIG_H

#define TOUCH_SENSOR1 1
// define more devices here as 2, 3, ... and provide config values for each device below

#if (DEVICE == TOUCH_SENSOR1)
  #define SERIAL_DEBUG            IN_USE 
  #define SECURITY                NOT_IN_USE // using security or not to encrypt messages
  #define MY_ROLE                 ESP_NOW_ROLE_IDLE  // This is reduntant for ESP32 and only applicable for ESP8266
  #define STATUS_LED              IN_USE // If Status LED is used or not, affects battery
  #define OTA                     IN_USE // If Status LED is used or not, affects battery
  #define DEVICE_NAME             "touch_sensor1" //max 15 characters without spaces
  uint8_t gatewayAddress[] =      GATEWAY_FF_AP_MAC; //comes from secrets.h
  #define WiFi_SSID               primary_ssid //from secrets.h
  #define WiFi_SSID_PSWD          primary_ssid_pswd // used only for OTA updates else this is not used , from secrets.h
  #define ESP_IP_ADDRESS          IP_espnow_sensor //from secrets.h\static_ipaddress.h
//  constexpr char WIFI_SSID[] =    primary_ssid;// from secrets.h , SSID to which the Slave connects for WiFi, only SSID name needed here, not password
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
  #define LED_GPIO                2
  #define LED_INVERTED            false // If LED is Active HIGH , define as false , if Active LOW , define as true
  #define LED_ON_DURATION         0 // Duration in millisecs for which Status LED is ON. If 0 then it will be ON for the lenght of ESP wakeup. 
                                  // Irrespective of the value specified , it will not be ON for a time less than ESP wake time , ~ 90ms
                                  // Suggest to keep this at 0 to save battery or even dont use LED 
#else
  #error "Device type not found. Have you passed DEVICE id in platform.ini as build flag. See Config.h for all DEVICES"
#endif

#if USING(SECURITY)
uint8_t kok[16]= PMK_KEY_STR;//comes from secrets.h
uint8_t key[16] = LMK_KEY_STR;// comes from secrets.h
#endif


#endif
