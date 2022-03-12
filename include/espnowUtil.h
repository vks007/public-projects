/*
This file defines all utility functions for a espnow device. 
It assumes the following constants/variables being defined in the main program
    WIFI_SSID
    MY_ROLE
    gatewayAddress

*/

#ifndef ESPNOW_UTIL_H
#define ESPNOW_UTIL_H
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <espnow.h>
#include "espnowMessage.h" // for struct of espnow message
#include "Debugutils.h" //This file is located in the Sketches\libraries\DebugUtils folder
#include <ESP_EEPROM.h>

#define WAIT_TIMEOUT 25 // time in millis to wait for acknowledgement of the message sent
#define MAX_MESSAGE_RETRIES 2 // No of times a message is retries to be sent before dropping the message
uint8_t channel = 0;//stores the channel of the slave by scanning the SSID the slave is on.
volatile uint8_t bResult = 9;
volatile bool bResultReady = false;
bool channelRefreshed = false;//tracks the status of the change in wifi channel , true -> wifi channel has been refreshed
// it is set to false every time the ESP boots

//Define the esp_now_peer_info if we're working with esp8266, for ESP32 its already defined
#ifdef ESP8266
typedef struct esp_now_peer_info {// this is defined in esp_now.h for ESP32 but not in espnow.h for ESP8266
  u8 peer_addr[6];
  uint8_t channel;
  uint8_t encrypt;
}esp_now_peer_info_t;
#endif

/*
 * Gets the WiFi channel of the SSID of your router, It has to be on the same channel as this one as the receiver will listen to ESPNow messages on the same 
 * While theritically it should be possible for WiFi and ESPNow to work on different channels but it seems due to some bug (or behavior) this isnt possible with espressif
 */
uint8_t getWiFiChannel(const char *ssid) {
  if (int32_t n = WiFi.scanNetworks()) {
      for (uint8_t i=0; i<n; i++) {
          if (!strcmp(ssid, WiFi.SSID(i).c_str())) {
              return WiFi.channel(i);
          }
      }
  }
  return 0;
}

/*
 * calls all statements to initialize the esp device for espnow messages
 * It sets STA mode, reads channel from RTC, if invalid, re-scans channel, sets ESP role
 * Params: forceChannelRefresh : true forces the channel to be scanned again , false: tries to read the channel from RTC memory, if it fails then scans afresh
 */
void Initilize_espnow(bool forceChannelRefresh = false, bool restartOnError= false)
{
  // Set device as a Wi-Fi Station and set channel
  WiFi.mode(WIFI_STA);

  if(!forceChannelRefresh)
  {
    if(EEPROM.get(0, channel))
    {
      DPRINTFLN("wifi channel read from memory = %d",channel);
    }
    else
      DPRINTLN("Failed to read wifi channel from memory");
  }
  if((channel <= 0 || channel > 14) || forceChannelRefresh)//we have an invalid channel, it can only range between 1-14 , scan for a valid channel
  {
      channel = getWiFiChannel(WIFI_SSID);
      if(channel != 0)
      {
        if(EEPROM.put(0, channel) && EEPROM.commit())
        {
          DPRINTFLN("wifi channel written to memory = %d",channel);
        }
        else
          DPRINTLN("Failed to write wifi channel from memory");
      }
      else
        DPRINTLN("Failed to get a valid channel for " && WIFI_SSID);
  }

  WiFi.disconnect(); // trying to see if the issue of sometimes it not setting the right channel gets solved by this.
  // To change the channel you have to first call wifi_promiscuous_enable(true) , change channel and then call wifi_promiscuous_enable(false)
  //  WiFi.printDiag(Serial);
  wifi_promiscuous_enable(true);
  wifi_set_channel(channel);
  wifi_promiscuous_enable(false);
  delay(10);
  uint8_t ch = wifi_get_channel();
  DPRINTFLN("channel:%d",ch);
  //strange behavior : If I define ch as byte and make the comparison below , the ESP resets due to WDT and then hangs
  if(ch == 0 && restartOnError)
  {
    DPRINTLN("WiFi Channel not set properly, restarting");
    ESP.restart();
  }
//  WiFi.printDiag(Serial);

  //Init ESP-NOW
  if (esp_now_init() != 0) {
    DPRINTLN("Error initializing ESP-NOW");
    return;
  }
  esp_now_set_self_role(MY_ROLE);
}

bool refreshPeer()
{
    esp_now_del_peer(gatewayAddress);//delete peer if it is present
    //Add peer , Note: There is no method in ESP8266 to add a peer by passing esp_now_peer_info_t object unlike ESP32
    if (esp_now_add_peer((u8*)gatewayAddress, RECEIVER_ROLE, channel, NULL, 0) != 0){
        DPRINTFLN("Failed to add peer on channel:%u",channel);
        return false;
    }
    DPRINTFLN("Added peer on channel:%u",channel);
    return true;
}

/*
 * Sends a espnow_message to the slave. It retries to send the message a certain no of times if if fails and then gives up
 * Relies on the variable bResultReady to set true in the call back function of the OnDataSent to determine if the message sending was successful
 * You must define this in the OnDataSent function in your code
 */
void send_espnow_message(espnow_message *myData)
{
  // try to send the message MAX_MESSAGE_RETRIES times if it fails
  for(short i = 0;i<MAX_MESSAGE_RETRIES;i++)
  {
    //DPRINTLN(millis());
    DPRINTLN(myData->intvalue3);
    int result = esp_now_send(gatewayAddress, (uint8_t *) myData, sizeof(*myData));
    // If devicename is not given then generate one from MAC address stripping off the colon
    long waitTimeStart = millis();
    if (result == 0) DPRINTLN("Sent with success");
    else DPRINTLN("Error sending the data");
    //get a confirmation of successful delivery , else will try again
    while(!bResultReady && ((millis() - waitTimeStart) < WAIT_TIMEOUT))
    {
      delay(1);
    }
    //DPRINTFLN("wait:%u",(millis() - waitTimeStart));
    bResultReady = false; // prepare for next iteration
    if(bResult)
      break;
    else
    {
        //See if we are on the right channel, it might have changed since last time we wrote the same in RTC memory
        // Scan for the WiFi channel again and store the new value in the RTC memory, Do it only once
        if(!channelRefreshed)
        {
            DPRINTLN("Re-initialize ESP to rfresh wifi channel");
            Initilize_espnow(true);
            channelRefreshed = true;// this will enable refreshing of channel only once in a cycle
        }
    }
  }

}

#endif