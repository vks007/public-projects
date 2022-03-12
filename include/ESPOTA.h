/*
 * ESPOTA.h - provides OTA capability for an ESP board (not tested with ESP32 yet)
 * To use this follow the following steps :
	- include this file in your program
	- include a call to setupOTA() in the setup after connecting to WiFi
	- include the call to the function ArduinoOTA.handle() in the main loop
*/

#ifndef ESPOTA_H
#define ESPOTA_H
#include <ArduinoOTA.h>
#include "Debugutils.h" 

#ifdef USE_OTA
#define SETUP_OTA() setupOTA()
#define HANDLE_OTA() ArduinoOTA.handle();

void setupOTA()
{
  ArduinoOTA.setHostname((const char *)WiFi.hostname().c_str());
  ArduinoOTA.onStart([]() {
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
  DPRINTLN("OTA Server setup successfully");
}

#else
	#define SETUP_OTA()
	#define HANDLE_OTA()
#endif
	
#endif
