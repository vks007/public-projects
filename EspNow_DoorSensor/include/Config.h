#ifndef CONFIG_H
#define CONFIG_H

// you cant use strings so you have to use #defines for numeric values
// Ref :https://stackoverflow.com/questions/2335888/how-to-compare-strings-in-c-conditional-preprocessor-directives
//The following defines the device type for which the program is being compiled, this is passed as a compile time parameter in platform.ini
#define MAIN_DOOR 1
#define TERRACE_DOOR 2
#define BALCONY_DOOR 3
#define TEST_DOOR 4

#define LOGIC_NORMAL  1 // logic to hold power with GPIO HIGH and cut off power with GPIO LOW
#define LOGIC_INVERTED 2 // logic to hold power with GPIO LOW and cut off power with GPIO HIGH

#if (DEVICE == MAIN_DOOR) 
  #pragma message "Compiling the program for the device: MAIN_DOOR"
  #define SERIAL_DEBUG            IN_USE 
  #define SECURITY                NOT_IN_USE // using security or not to encrypt messages
  #define DEVICE_NAME             "main_door"
  #define HOLD_PIN 0  // defines hold pin (will hold power to the ESP).
  #define SIGNAL_PIN 3 //indicates the message type
  //State Mapping of SIGNAL_PIN0 SIGNAL_PIN1:: 11=>IDLE , 00=> SENSOR_WAKEUP , 01=> SENSOR OPEN , 10=> SENSOR CLOSED
  #define MY_ROLE         ESP_NOW_ROLE_COMBO              // set the role of this device: CONTROLLER, SLAVE, COMBO
  #define RECEIVER_ROLE   ESP_NOW_ROLE_COMBO              // set the role of the receiver
  uint8_t gatewayAddress[] = GATEWAY_FF_AP_MAC; //comes from secrets.h
  constexpr char WIFI_SSID[] = primary_ssid;// from secrets.h
  #define HOLDING_LOGIC LOGIC_NORMAL

#elif (DEVICE == TERRACE_DOOR)
  #pragma message "Compiling the program for the device: TERRACE_DOOR"
  #define SERIAL_DEBUG            IN_USE 
  #define SECURITY                NOT_IN_USE // using security or not to encrypt messages
  #define DEVICE_NAME             "terrace_door" // This becomes the postfix of the final MQTT topic under which messages are published
  #define HOLD_PIN 0  // defines hold pin (will hold power to the ESP).
  #define SIGNAL_PIN 3 //indicates the message type
  //State Mapping of SIGNAL_PIN0 SIGNAL_PIN1:: 11=>IDLE , 00=> SENSOR_WAKEUP , 01=> SENSOR OPEN , 10=> SENSOR CLOSED
  #define MY_ROLE         ESP_NOW_ROLE_COMBO              // set the role of this device: CONTROLLER, SLAVE, COMBO
  #define RECEIVER_ROLE   ESP_NOW_ROLE_COMBO              // set the role of the receiver
  // gateway MAC Address , This should be the address of the softAP (and NOT WiFi MAC addr obtained by WiFi.macAddress()) if the Receiver uses both, WiFi & ESPNow
  // You can get the address via the command WiFi.softAPmacAddress() , usually it is one decimal no after WiFi MAC address
  // As a best practice you should define your own custom Soft MAC address so that you dont have to update all your sensors if you change the gateway device
  uint8_t gatewayAddress[] = GATEWAY_FF_AP_MAC; //comes from secrets.h
  constexpr char WIFI_SSID[] = primary_ssid;// from secrets.h
  #define HOLDING_LOGIC LOGIC_NORMAL

#elif (DEVICE == BALCONY_DOOR)
  #pragma message "Compiling the program for the device: BALCONY_DOOR"
  #define SERIAL_DEBUG            IN_USE 
  #define SECURITY                NOT_IN_USE // using security or not to encrypt messages
  #define DEVICE_NAME             "balcony_door"
  #define HOLD_PIN 5  // defines hold pin (will hold power to the ESP).
  #define SIGNAL_PIN 4 //indicates the message type
  #define MY_ROLE         ESP_NOW_ROLE_COMBO              // set the role of this device: CONTROLLER, SLAVE, COMBO
  #define RECEIVER_ROLE   ESP_NOW_ROLE_COMBO              // set the role of the receiver
  uint8_t gatewayAddress[] = GATEWAY_FF_AP_MAC; //comes from secrets.h
  constexpr char WIFI_SSID[] = primary_ssid;// from secrets.h
  #define BOUNCE_DELAY 1 // bounce delay in seconds, this is used for a bumby door which bounces a few times before settling on either open or closed
  #define HOLDING_LOGIC LOGIC_INVERTED

#elif (DEVICE == TEST_DOOR)
  #pragma message "Compiling the program for the device: TEST_DOOR" 
  #define SERIAL_DEBUG            IN_USE 
  #define SECURITY                NOT_IN_USE // using security or not to encrypt messages
  #define DEVICE_NAME             "test_door"
  #define HOLD_PIN 5  // defines hold pin (will hold power to the ESP).
  #define SIGNAL_PIN 4 //indicates the message type
  #define MY_ROLE         ESP_NOW_ROLE_CONTROLLER              // set the role of this device: CONTROLLER, SLAVE, COMBO
  #define RECEIVER_ROLE   ESP_NOW_ROLE_SLAVE              // set the role of the receiver
  uint8_t gatewayAddress[] = GATEWAY_FF_AP_MAC; //comes from secrets.h
  constexpr char WIFI_SSID[] = primary_ssid;// from secrets.h
  #define BOUNCE_DELAY 1 // bounce delay in seconds, this is used for a bumby door which bounces a few times before settling on either open or closed
  #define HOLDING_LOGIC LOGIC_NORMAL


#else
  #error "Device type not selected, see Door_config.h"
#endif

#endif
