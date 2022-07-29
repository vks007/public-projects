/*
 *
*/
//Specify the sensor this is being compiled for in platform.ini, see Config.h for list of all devices this can be compiled for

// ************ HASH DEFINES *******************
#define EEPROM_SIZE 16 // number of bytes to be allocated to EEPROM
#define MSG_WAIT_TIMEOUT 30 // time in ms to wait for receiving any incoming messages to this ESP , typically 10-40 ms
#define OTA_TIMEOUT 60 // time in seconds beyond which to come out of OTA mode
#define VERSION "1.1.0"
// ************ HASH DEFINES *******************

#include <Arduino.h>
#include "macros.h"
#include "secrets.h"
#include "Config.h"
#include "Debugutils.h" //This file is located in the Sketches\libraries\DebugUtils folder
#include <esp_now.h>
#include "espnowMessage.h" // for struct of espnow message
#include "myutils.h"
#include <EEPROM.h> // to store WiFi channel number to EEPROM
#include <ArduinoOTA.h> 
#include "version.h" // this defines a variable compile_version which gives the complete version of the program

// ************ GLOBAL OBJECTS/VARIABLES *******************
const char* ssid = WiFi_SSID; // comes from config.h
const char* password = WiFi_SSID_PSWD; // comes from config.h
espnow_message myData;
char device_id[13];
esp_now_peer_info_t peerInfo; // This object must be a global object else the setting of peer will fail
int gpio_pin = 100;
volatile bool msgReceived = false; //flag to indicate if the ESP has received any message during its wake up cycle
volatile bool ota_msg = false; // indicates if the esp has received a OTA message
volatile bool ota_mode = false; // determines if the ESP should start in the OTA mode or ESPNOW mode
unsigned long start_time = millis(); // keeps track of the time ESP started, can be changed in between though
// ************ GLOBAL OBJECTS/VARIABLES *******************
// need to include this file after ssid variable as I am using ssid inside espcontroller, not a good design but will sort this out later
#include "espnowController.h" //defines all utility functions for sending espnow messages from a controller

// MAC Address , This should be the address of the softAP (and NOT WiFi MAC addr obtained by WiFi.macAddress()) if the Receiver uses both, WiFi & ESPNow
// You can get the address via the command WiFi.softAPmacAddress() , usually it is one decimal no after WiFi MAC address

/*
 * Callback when data is sent , It sets the bResultReady flag to true on successful delivery of message
 * The flag is set to false in the main loop where data is sent and then the code waits to see if it gets set to true, if not it retires to send
 */
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  deliverySuccess = status;
  DPRINT("OnDataSent:Last Packet delivery status:\t");
  DPRINTLN(status == 0 ? "Success" : "Fail");
  bResultReady = true;
}

/*
 * Callback called when a message is received , nothing to do here for now , just log message
 */
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  espnow_message msg;
  memcpy(&msg, incomingData, sizeof(msg));
  if(!ota_msg && !ota_mode) // dont receive any more messages if we are already in OTA mode
  {
    msgReceived = true;
    if(msg.msg_type == ESP_NOW_OTA)
      ota_msg = true;
    DPRINTF("Processing msg:%lu,%u,%d,%d,%d,%d,%f,%f,%f,%f,%s,%s\n",msg.message_id,msg.msg_type,msg.intvalue1,msg.intvalue2,msg.intvalue3,msg.intvalue4,msg.floatvalue1,msg.floatvalue2,msg.floatvalue3,msg.floatvalue4,msg.chardata1,msg.chardata2);
  }
  else
    DPRINTF("Ignoring msg:%lu,%u,%d,%d,%d,%d,%f,%f,%f,%f,%s,%s\n",msg.message_id,msg.intvalue1,msg.intvalue2,msg.intvalue3,msg.intvalue4,msg.floatvalue1,msg.floatvalue2,msg.floatvalue3,msg.floatvalue4,msg.chardata1,msg.chardata2);
};

void print_init_info()
{
  DPRINTFLN("Version:%s",compile_version);
  DPRINTLN("Starting up ESPNow Touch module");
  #if USING(SECURITY)
    DPRINTLN("Security ON");
  #else
    DPRINTLN("Security OFF");
  #endif
  String wifiMacString = WiFi.macAddress();
  DPRINTFLN("This device's MAC add: %s",wifiMacString.c_str());

}

void setup_OTA()
{
  WiFi.mode(WIFI_AP_STA);
  WiFi.config(ESP_IP_ADDRESS, default_gateway, subnet_mask);//from secrets.h
  String device_name = DEVICE_NAME;
  device_name.replace("_","-");//hostname dont allow underscores or spaces
  WiFi.hostname(device_name.c_str());// Set Hostname.
  WiFi.begin(ssid, password);
  DPRINTLN("Setting as a Wi-Fi Station..");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    DPRINT(".");
  }
  DPRINT("Station IP Address: ");
  DPRINTLN(WiFi.localIP());
  WiFi.setAutoReconnect(true);
  
  ArduinoOTA.onStart([]() {
    start_time = millis();//reset the start time now that we've started OTA
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    DPRINTLN("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    DPRINTLN("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    DPRINTF("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    DPRINTF("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      DPRINTLN("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      DPRINTLN("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      DPRINTLN("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      DPRINTLN("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      DPRINTLN("End Failed");
    }
  });
  ArduinoOTA.begin();
  // Now that we've started in OTA mode, clear the flag in EEPROM so that we start up in ESPNOW mode the next time
  // This is irrespective of whether OTA setup completes successfully or not
  EEPROM.write(sizeof(int),(byte)false);
  EEPROM.commit();
  DPRINTLN("OTA set up successfully");

}

void setup() {
  //Init Serial Monitor
  DBEGIN(115200);
  DPRINTLN();
  #if(USING(STATUS_LED))
    pinMode(LED_GPIO,OUTPUT);
    digitalWrite(LED_GPIO,LED_INVERTED?LOW:HIGH); // turn ON the LED
  #endif
  
  print_init_info();
  EEPROM.begin(EEPROM_SIZE);
  ota_mode = EEPROM.get(sizeof(int),ota_mode);
  DPRINTFLN("Starting up in %s mode",ota_mode?"OTA":"ESPNOW");
  if(ota_mode)
  {
    setup_OTA();
  }
  else
  {
    DPRINTLN("initializing ESPNOW");
    initilizeESP(ssid,MY_ROLE);

    #if(USING(SECURITY))
      esp_now_set_kok(kok, 16);
    #endif

    // register callbacks for events when data is sent and data is received
    esp_now_register_send_cb(OnDataSent);
    esp_now_register_recv_cb(OnDataRecv);

    memcpy(peerInfo.peer_addr, gatewayAddress, 6);
    #if(USING(SECURITY))
      peerInfo.encrypt = true;
      peerInfo.lmk = key;
    #else
      peerInfo.encrypt = false;
    #endif
    refreshPeer(&peerInfo);
    
    touch_pad_t touchPin;
    touchPin = esp_sleep_get_touchpad_wakeup_status();
    DPRINTFLN("TouchPin %u",touchPin);
    switch(touchPin)
    {
      case 0  : {    gpio_pin = 4;DPRINTLN("Touch detected on GPIO 4"); break;  }
      case 1  : {    gpio_pin = 0;DPRINTLN("Touch detected on GPIO 0"); break;  }
      case 2  : {    gpio_pin = 2;DPRINTLN("Touch detected on GPIO 2"); break;  }
      case 3  : {    gpio_pin = 15;DPRINTLN("Touch detected on GPIO 15"); break;  }
      case 4  : {    gpio_pin = 13;DPRINTLN("Touch detected on GPIO 13"); break;  }
      case 5  : {    gpio_pin = 12;DPRINTLN("Touch detected on GPIO 12"); break;  }
      case 6  : {    gpio_pin = 14;DPRINTLN("Touch detected on GPIO 14"); break;  }
      case 7  : {    gpio_pin = 27;DPRINTLN("Touch detected on GPIO 27"); break;  }
      case 8  : {    gpio_pin = 33;DPRINTLN("Touch detected on GPIO 33"); break;  }
      case 9  : {    gpio_pin = 32;DPRINTLN("Touch detected on GPIO 32"); break;  }
      default : {    DPRINTLN("Not a pin wakeup, probably starting up"); break;  }
    }
    DPRINTFLN("gpio_pin %u",gpio_pin);
  }
}

void callback0(){}
void callback1(){}
void callback2(){}
void callback3(){}
void callback4(){}
void callback5(){}
void callback6(){}
void callback7(){}
void callback8() {}
void callback9(){}

/*
* Puts the ESP into deep sleep with touch wakeup
*/
void go_to_sleep()
{
  //Setup interrupt on Touch pins
  // touchAttachInterrupt(TOUCHPIN0, callback0, THRESHOLD);
  // touchAttachInterrupt(TOUCHPIN1, callback1, THRESHOLD);
  // touchAttachInterrupt(TOUCHPIN2, callback2, THRESHOLD);
  // touchAttachInterrupt(TOUCHPIN3, callback3, THRESHOLD);
  // touchAttachInterrupt(TOUCHPIN4, callback4, THRESHOLD);
  touchAttachInterrupt(TOUCHPIN5, callback5, THRESHOLD);
  touchAttachInterrupt(TOUCHPIN6, callback6, THRESHOLD);
  // touchAttachInterrupt(TOUCHPIN7, callback7, THRESHOLD);
  // touchAttachInterrupt(TOUCHPIN8, callback8, THRESHOLD);
  // touchAttachInterrupt(TOUCHPIN9, callback9, THRESHOLD);
  esp_sleep_enable_touchpad_wakeup();
  DPRINTFLN("Going to sleep, waiting for touch...");
  DFLUSH();
  esp_deep_sleep_start();
  DPRINTLN("In Deep sleep"); // This will never be printed

}

/*
* All sensor related work in the loop() is done in this function
*/
void send_message()
{
  if(gpio_pin >= 0 && gpio_pin <= 40)
  {
    //DPRINTFLN("Touch detected on GPIO %u",gpio_pin);
    myData.intvalue1 = gpio_pin;

    //Set other values to send
    // If devicename is not given then generate one from MAC address stripping off the colon
    #ifndef DEVICE_NAME
      String wifiMacString = WiFi.macAddress();
      wifiMacString.replace(":","");
      snprintf(myData.device_name, 16, "%s", wifiMacString.c_str());
    #else
      strcpy(myData.device_name,DEVICE_NAME);
    #endif
    myData.intvalue2 = 0;
    myData.intvalue3 = 0;
    myData.intvalue4 = 0;
    myData.floatvalue1 = 0;
    myData.floatvalue2 = 0;
    myData.floatvalue3 = 0;
    myData.floatvalue4 = 0;
    strcpy(myData.chardata1,"");
    strcpy(myData.chardata2,"");
    myData.message_id = millis();//there is no use of message_id so using it to send the uptime
      
    //int result = esp_now_send(gatewayAddress, (uint8_t *) &myData, sizeof(myData));
    int result = sendESPnowMessage(&myData,gatewayAddress);
    if (result == 0) {
      DPRINTLN("Delivered with success");}
    else {DPRINTFLN("Error sending/receipting the message, error code:%d",result);}
  }
  else
  {
    DPRINTLN("nothing to do");
  }
}

/*
* Turns LED off after a predermined total time, kills time via delay() if ESP hasnt been ON for that certain time
*/
void set_led_off()
{
  #if(USING(STATUS_LED))
    // kill time to keep LED ON if its ON duration is > the time elasped till yet
    if(LED_ON_DURATION != 0)
    {
      while(millis() < LED_ON_DURATION)
      {
        delay(1);yield();
      }
    }
    digitalWrite(LED_GPIO,LED_INVERTED?HIGH:LOW); // turn OFF the LED
  #endif
}

/*
* Processes incoming messages, for now its only OTA msg type, In future can code for more events
* For future events I will have to collect incoming messages in a queue and then process them
*/
void process_messages()
{
  if(ota_msg)
  {
    // write true in EEPROM and restart the ESP
    EEPROM.write(sizeof(int),(byte)true);
    EEPROM.commit();
    DPRINTFLN("OTA Flag read back from EEPROM %u",EEPROM.get(sizeof(int),ota_mode));
    DPRINTFLN("Going to restart the ESP for OTA mode...");
    DFLUSH();
    ESP.restart();
  }
  msgReceived = false; //now that we've processed the message , clear the flag
  // You can code for future events here
}

/*
* scan for received messages
*/
void scan_for_messages()
{
  // Wait for some time to see if we haev any service message for this ESP
  for(byte i=0;i<MSG_WAIT_TIMEOUT;i++)
  {
    delay(1);
    if(msgReceived)
    {
      process_messages();
      break;
    }
    yield();
  }
}

void loop() {
  if(ota_mode) 
  {
    ArduinoOTA.handle();
    // countdown to the max time you should remain in the OTA mode before going back to sleep
    if((millis()- start_time) > OTA_TIMEOUT*1000)
    {
      set_led_off();
      DPRINTLN("Timing out of OTA mode");
      ota_mode = false; // not necessary but anyway
      go_to_sleep();
    }
  }
  else 
  {
    send_message();
    scan_for_messages();
    if(!msgReceived) //no messages are received to process, turn off led and go to sleep
    {
      set_led_off();
      go_to_sleep();
    }
    //else it will go back to the loop() and process OTA or other messages
  }

}
