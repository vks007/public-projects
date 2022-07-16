/*
 *
*/
//Specify the sensor this is being compiled for in platform.ini, see Config.h for list of all devices this can be compiled for
#include <Arduino.h>
#include "macros.h"
#include "secrets.h"
#include "Config.h"
#include "Debugutils.h" //This file is located in the Sketches\libraries\DebugUtils folder
#include <esp_now.h>
#include "espnowMessage.h" // for struct of espnow message
#include "myutils.h"
#include <EEPROM.h> // to store WiFi channel number to EEPROM
#include "espnowController.h" //defines all utility functions for sending espnow messages from a controller

// ************ GLOBAL OBJECTS/VARIABLES *******************
espnow_message myData;
char device_id[13];
esp_now_peer_info_t peerInfo; // This object must be a global object else the setting of peer will fail
// ************ GLOBAL OBJECTS/VARIABLES *******************

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
  DPRINTF("OnDataRecv:%lu,%d,%d,%d,%d,%f,%f,%f,%f,%s,%s\n",msg.message_id,msg.intvalue1,msg.intvalue2,msg.intvalue3,msg.intvalue4,msg.floatvalue1,msg.floatvalue2,msg.floatvalue3,msg.floatvalue4,msg.chardata1,msg.chardata2);
};

void printInitInfo()
{
  DPRINTLN("Starting up as a ESPNow Temperature sensor");
  #if USING(SECURITY)
    DPRINTLN("Security ON");
  #else
    DPRINTLN("Security OFF");
  #endif
  String wifiMacString = WiFi.macAddress();
  DPRINTFLN("This device's MAC add: %s",wifiMacString.c_str());

}

void setup() {
  //Init Serial Monitor
  DBEGIN(115200);
  DPRINTLN();
  printInitInfo();
  EEPROM.begin(EEPROM_SIZE);

  DPRINTLN("initializing ESPNOW");
  initilizeESP(WIFI_SSID,MY_ROLE);

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

void loop() {
    touch_pad_t touchPin;
    touchPin = esp_sleep_get_touchpad_wakeup_status();
    int gpio_pin = -1;
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

    if(gpio_pin != -1)    
    {
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
    DPRINTFLN("Going to sleep, waiting for touch...");
    DFLUSH();
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
    esp_deep_sleep_start();
    DPRINTFLN("In Deep sleep"); // This will never be printed

}
