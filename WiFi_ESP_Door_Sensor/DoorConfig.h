
#ifndef DOOR_CONFIG_H
#define DOOR_CONFIG_H

IPAddress mqtt_server = mqtt_broker ; //from secrets.h
const char mqtt_user[] = mqtt_uname; //from secrets.h
const char mqtt_password[] = mqtt_pswd; //from secrets.h
const short mqtt_port_no = mqtt_port;//from secrets.h


#if defined(MAIN_DOOR)
  #warning "Compiling the program for the device: MAIN_DOOR"
  #define DEVICE_NAME             "main_door"
  #define MQTT_TOPIC              "home/main_door"
  #define ESP_IP_ADDRESS          IPAddress(192,168,1,50)
  #define WiFi_SSID               gf_ssid //from secrets.h
  #define WiFi_SSID_PSWD          gf_ssid_pswd //from secrets.h
  #define HOLD_PIN 0  // defines the pin which holds power to the ESP
  #define SIGNAL_PIN 1 //Bit 1 of the signal which indicates the message type

#elif defined(TERRACE_DOOR)
  #warning "Compiling the program for the device: TERRACE_DOOR"
  #define DEVICE_NAME             "terrace_door"
  #define MQTT_TOPIC              "home/terrace_door"
  #define ESP_IP_ADDRESS          IPAddress(192,168,1,51)
  #define WiFi_SSID               primary_ssid //from secrets.h
  #define WiFi_SSID_PSWD          primary_ssid_pswd //from secrets.h
  #define HOLD_PIN 0  // defines the pin which holds power to the ESP
  #define SIGNAL_PIN 1 //Bit 1 of the signal which indicates the message type

#elif defined(BALCONY_DOOR)
  #warning "Compiling the program for the device: BALCONY_DOOR"
  #define DEVICE_NAME             "balcony_door"
  #define MQTT_TOPIC              "home/balcony_door"
  #define ESP_IP_ADDRESS          IPAddress(192,168,1,52)
  #define WiFi_SSID               primary_ssid //from secrets.h
  #define WiFi_SSID_PSWD          primary_ssid_pswd //from secrets.h
  #define HOLD_PIN 5  // defines the pin which holds power to the ESP
  #define SIGNAL_PIN 4 //the signal which indicates the message type
#else
  #error "Device type not selected, see Door_config.h"
#endif

#endif
