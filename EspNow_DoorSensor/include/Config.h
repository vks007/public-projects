#ifndef CONFIG_H
#define CONFIG_H

// you cant use strings so you have to use #defines for numeric values
// Ref :https://stackoverflow.com/questions/2335888/how-to-compare-strings-in-c-conditional-preprocessor-directives
//The following defines the device type for which the program is being compiled, this is passed as a compile time parameter in platform.ini
#define MAIN_DOOR 1
#define TERRACE_DOOR 2
#define BALCONY_DOOR 3

#if (DEVICE == MAIN_DOOR) 
  #pragma message "Compiling the program for the device: MAIN_DOOR"
  #define DEVICE_NAME             "main_door"
  //Hold pin will hold CH_PD HIGH till we're executing the setup, the last step would be set it LOW which will power down the ESP
  #define HOLD_PIN 0  // defines GPIO0 as the hold pin (will hold CH_PD high untill we power down).
  #define SIGNAL_PIN 3 //Bit 2 of the signal which indicates the message type
  //State Mapping of SIGNAL_PIN0 SIGNAL_PIN1:: 11=>IDLE , 00=> SENSOR_WAKEUP , 01=> SENSOR OPEN , 10=> SENSOR CLOSED
  #define MY_ROLE         ESP_NOW_ROLE_COMBO              // set the role of this device: CONTROLLER, SLAVE, COMBO
  #define RECEIVER_ROLE   ESP_NOW_ROLE_COMBO              // set the role of the receiver
  uint8_t gatewayAddress[] = GATEWAY_GF_MAC; //comes from secrets.h
  constexpr char WIFI_SSID[] = gf_ssid;// from secrets.h

#elif (DEVICE == TERRACE_DOOR)
  #pragma message "Compiling the program for the device: TERRACE_DOOR"
  #define DEVICE_NAME             "terrace_door" // This becomes the postfix of the final MQTT topic under which messages are published
  //Hold pin will hold CH_PD HIGH till we're executing the setup, the last step would be set it LOW which will power down the ESP
  #define HOLD_PIN 0  // defines GPIO0 as the hold pin (will hold CH_PD high untill we power down).
  #define SIGNAL_PIN 3 //Bit 2 of the signal which indicates the message type
  //State Mapping of SIGNAL_PIN0 SIGNAL_PIN1:: 11=>IDLE , 00=> SENSOR_WAKEUP , 01=> SENSOR OPEN , 10=> SENSOR CLOSED
  #define MY_ROLE         ESP_NOW_ROLE_COMBO              // set the role of this device: CONTROLLER, SLAVE, COMBO
  #define RECEIVER_ROLE   ESP_NOW_ROLE_COMBO              // set the role of the receiver
  // gateway MAC Address , This should be the address of the softAP (and NOT WiFi MAC addr obtained by WiFi.macAddress()) if the Receiver uses both, WiFi & ESPNow
  // You can get the address via the command WiFi.softAPmacAddress() , usually it is one decimal no after WiFi MAC address
  uint8_t gatewayAddress[] = GATEWAY_FF_MAC; //comes from secrets.h
  constexpr char WIFI_SSID[] = primary_ssid;// from secrets.h

#elif (DEVICE == BALCONY_DOOR)
  #pragma message "Compiling the program for the device: BALCONY_DOOR"
  #define DEVICE_NAME             "balcony_door"
  #define HOLD_PIN 5  // defines hold pin (will hold CH_PD high untill we power down).
  #define SIGNAL_PIN 4 //Bit 2 of the signal which indicates the message type
  #define MY_ROLE         ESP_NOW_ROLE_COMBO              // set the role of this device: CONTROLLER, SLAVE, COMBO
  #define RECEIVER_ROLE   ESP_NOW_ROLE_COMBO              // set the role of the receiver
  uint8_t gatewayAddress[] = GATEWAY_FF_MAC; //comes from secrets.h
  constexpr char WIFI_SSID[] = primary_ssid;// from secrets.h

#else
  #error "Device type not selected, see Door_config.h"
#endif

#endif
