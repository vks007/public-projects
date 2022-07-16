/*
 * This is a slave (gateway) implemented using a ESP8266 which listens to espnow messages from other controllers (masters/sensors) and passes on these messages to MQTT
 * Some points to keep in mind:
 * The gateway connects to an AP (WiFi router) and also listens to espnow messages and this forces the ESP to be on a single channel dictated by the router. Hence the espnow channel is also dictated
 * This means that the masters also have to operate ont he same channel. You can get the channel on the master by scanning the SSID of the router and determining its channel. 
 * Features:
 * - Implements both the WiFi station and ESPNow slave in a single ESP module
 * - Retries MQTT connection a few times before giving up
 * - Publishes initial health message on startup and then a health message at a set interval, stats like msg count, msg rate, queue length, free memory, uptime etc are posted
 * - Supports OTA
 * - DONE - Do not pop out the message form the queue in case posting to MQTT isnt successful
 * 
 * TO DO :
 * - encryption isnt working. Even if I change the keys on the master to random values, the slave is able to receieve the messages, so have to debug later
 *  if you can solve the encryption issue, remove it as with excryption , an eSP8266 can only connect to 6 other peers, ESP32 can connect to 10 other
 *  while without encryption they can connect to 20 peers, encryption eg from :https://github.com/espressif/ESP8266_NONOS_SDK/issues/114#issuecomment-383521100
 * - Change the role to COMBO for both slave and Controller so that Slave can also pass on administration messages to the controller.
 * - construct the controller topic from its mac address instead of picking it up from the message id. Instead use message id as a string to identify the device name
 * - Introduce a status LED for MQTT connection status
 * - Implement a restart of ESP after configurable interval if the connection to MQTT is not restored
 * - To Support COMBO role where it can receive and send messages
 */

// IMPORTANT : Compile it for the device you want, details of which are in Config.h
// Define macros for turning features ON and OFF , usage : #define SECURITY IN_USE

//#define DEBUG (1) //BEAWARE that this statement should be before #include "Debugutils.h" else the macros wont work as they are based on this #define
/* For now currently turning OFF security as I am not able to make it work. It works even if the keys aren't the same on controller and slave
 * Also I have to find a way to create a list of multiple controllers as with security you haev to register each controller separately
 * See ref code here: https://www.electrosoftcloud.com/en/security-on-your-esp32-with-esp-now/
*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "Config.h"
#include "Debugutils.h" //This file is located in the Sketches\libraries\DebugUtils folder
#include <espnow.h>
#include "secrets.h"
#include <ArduinoJson.h>
#include <ArduinoQueue.h>
#include <ArduinoOTA.h>
#include "espnowMessage.h" // for struct of espnow message
#include <PubSubClient.h>
#include <Pinger.h>
#include "myutils.h"
#include "espwatchdog.h"
#include "pir_sensor.h"
#include <ezLED.h> // I am using a local copy of this library instead from the std library repo as that has an error in cpp file.
// The author has defined all functions which take a default argumnent again in the cpp file whereas the default argument should only be 
// specified in the decleration and not in the implementation of the function.


#define VERSION "1.2"
const char compile_version[] = VERSION " " __DATE__ " " __TIME__; //note, the 3 strings adjacent to each other become pasted together as one long string
#define KEY_LEN  16 // lenght of PMK & LMK key (fixed at 16 for ESP)
#define MQTT_RETRY_INTERVAL 5000 //MQTT server connection retry interval in milliseconds
#define QUEUE_LENGTH 50
#define MAX_MESSAGE_LEN 251 // defnies max message length, as the max espnow allows is 250, cant exceed it
#define ESP_OK 0 // This is defined for ESP32 but not for ESP8266 , so define it
#define HEALTH_INTERVAL 30e3 // interval is millisecs to publish health message for the gateway
// API_TIMEOUT is used to determine how long to wait for MQTT connection before restarting the ESP
#ifndef API_TIMEOUT
  #define API_TIMEOUT 600 // define default timeout of monitoring for MQTT connection if not defined.
#endif
const char* ssid = WiFi_SSID;
const char* password = WiFi_SSID_PSWD;
long last_time = 0;
bool retry_message = false;
long last_message_count = 0;//stores the last count with which message rate was calculated
long message_count = 0;//keeps track of total no of messages publshed since uptime
long lastReconnectAttempt = 0; // Keeps track of the last time an attempt was made to connect to MQTT
bool initilised = false; // flag to track if initialisation of the ESP has finished. At present it only handles tracking of the "init" message published on startup
String strIP_address = "";//stores the IP address of the ESP

extern "C"
{
  #include <lwip/icmp.h> // needed for icmp packet definitions
}
// Set global to avoid object removing after setup() routine
Pinger pinger;
ezLED  statusLED(STATUS_LED);
watchDog MQTT_wd = watchDog(API_TIMEOUT); // monitors the MQTT connection, if it is disconnected beyond API_TIMEOUT , it restarts ESP

//List of controllers(sensors) who will send messages to this receiver
uint8_t controller_mac[][6] = CONTROLLERS; //from secrets.h
//example entry : 
// uint8_t controller_mac[2][6] = {  
//    {0x4C, 0xF2, 0x32, 0xF0, 0x74, 0x2D} ,
//    {0xC, 0xDD, 0xC2, 0x33, 0x11, 0x98}
// };

uint8_t kok[KEY_LEN]= PMK_KEY_STR;//comes from secrets.h
uint8_t key[KEY_LEN] = LMK_KEY_STR;// comes from secrets.h

ArduinoQueue<espnow_message> structQueue(QUEUE_LENGTH);
WiFiClient espClient;
PubSubClient client(espClient);
static espnow_message emptyMessage;
espnow_message currentMessage = emptyMessage;

#if USING(MOTION_SENSOR)
pir_sensor motion_sensor(PIR_PIN,MOTION_ON_DURATION);
#endif

typedef enum
{
  MOTION = 0,
  UNDEFINED = 1
}msgType;

/*
 * connects to MQTT server , publishes LWT message as "online" every time it connects
 * waits for a time defined by MQTT_RETRY_INTERVAL before reconnecting again
 * I had issues with MQTT nto able to connect at times and so I included pinger to ping the gateway but then the issue hasnt happened for a long time now
 */
bool reconnectMQTT()
{
  if (!client.connected())
  {
    long now = millis();
    if ((now - lastReconnectAttempt > MQTT_RETRY_INTERVAL) || lastReconnectAttempt == 0) 
    {
      lastReconnectAttempt = now;
      DPRINTLN("Attempting MQTT connection...");
      // publishes the LWT message ("online") to the topic,If the this device disconnects from the broker ungracefully then the broker automatically posts the "offline" message on the LWT topic
      // so that all connected clients know that this device has gone offline
      char publish_topic[65] = ""; //variable accomodates 50 characters of main topic + 15 char of sub topic
      strcpy(publish_topic,MQTT_TOPIC);
      strcat(publish_topic,"/LWT"); 
      if (client.connect(DEVICE_NAME,mqtt_uname,mqtt_pswd,publish_topic,0,true,"offline")) {//credentials come from secrets.h
        DPRINTLN("MQTT connected");
        client.publish(publish_topic,"online",true);
        if(statusLED.getState() == LED_BLINKING)
          statusLED.cancel();
        return true;
      }
    //   else 
    //   {
    //     DPRINT("failed, rc=");
    //     DPRINT(client.state());
    //     if(pinger.Ping(WiFi.gatewayIP()))
    //     {
    //       DPRINTLN("ping to gateway success");
    //     }
    //     else
    //     {
    //       DPRINTLN("ping to gateway failed");
    //     }
    //   }
    }
    if(statusLED.getState() == LED_IDLE)
      statusLED.blink(1000, 500);
    return false;
  }
  if(statusLED.getState() == LED_BLINKING)
    statusLED.cancel();
  return true;
}

/*
 * publishes a string to MQTT
 */
bool publishToMQTT(const char msg[],const char topic[], bool retain)
{
  if(client.connected())
  {
    if(client.publish(topic,msg,retain))
    {
      DPRINT("publishToMQTT-Published:");DPRINTLN(msg);
      return true;
    }
    else
    {
      DPRINT("publishToMQTT-Failed:");DPRINTLN(msg);
    }
  }
  DPRINTLN("Publish failed - MQTT not connected");
  return false;
}

/*
 * Creates a json string from the espnow message and publishes it to a MQTT queue
 * Returns true if message was published successfully else false
 */
bool publishToMQTT(espnow_message msg) {
  char final_publish_topic[65] = "";
  char publish_topic[65] = "";
  // create a path for this specific device which is of the form MQTT_BASE_TOPIC/<master_id> , master_id is usually the mac address stripped off the colon eg. MQTT_BASE_TOPIC/2CF43220842D
  strcpy(publish_topic,MQTT_BASE_TOPIC);
  strcat(publish_topic,"/");
  //DPRINT("master_id:");DPRINTLN(msg.master_id);
  strcat(publish_topic,msg.device_name); // from here on all messages would be published within this topic specific for this device
  strcpy(final_publish_topic,publish_topic);
  strcat(final_publish_topic,"/state");// create a topic to publish the state of the device
  //DPRINT("Final topic:");DPRINTLN(final_publish_topic);
  DPRINTF("publishToMQTT:%lu,%d,%d,%d,%d,%f,%f,%f,%f,%s,%s\n",msg.message_id,msg.intvalue1,msg.intvalue2,msg.intvalue3,msg.intvalue4,msg.floatvalue1,msg.floatvalue2,msg.floatvalue3,msg.floatvalue4,msg.chardata1,msg.chardata2);
  
  StaticJsonDocument<MAX_MESSAGE_LEN> msg_json;
  msg_json["id"] = msg.message_id;
  msg_json["device"] = msg.device_name;
  msg_json["ival1"] = msg.intvalue1;
  msg_json["ival2"] = msg.intvalue2;
  msg_json["ival3"] = msg.intvalue3;
  msg_json["ival4"] = msg.intvalue4;
  msg_json["fval1"] = msg.floatvalue1;
  msg_json["fval2"] = msg.floatvalue2;
  msg_json["fval3"] = msg.floatvalue3;
  msg_json["fval4"] = msg.floatvalue4;
  msg_json["char1"] = msg.chardata1;
  msg_json["char2"] = msg.chardata2;
  
  String str_msg="";
  serializeJson(msg_json,str_msg);
  return publishToMQTT(str_msg.c_str(),final_publish_topic,false);
  //DPRINTF("published message with len:%u\n",measureJson(msg_json));
}

/*
 * Creates a message string for motion ON message and publishes it to a MQTT queue
 * Returns true if message was published successfully else false
 */
bool publishMotionMsgToMQTT(const char topic_name[],const char state[4]) {
  char publish_topic[65] = "";
  // create a path for this specific device which is of the form MQTT_BASE_TOPIC/<master_id> , master_id is usually the mac address stripped off the colon eg. MQTT_BASE_TOPIC/2CF43220842D
  strcpy(publish_topic,MQTT_TOPIC);
  strcat(publish_topic,"/");
  strcat(publish_topic,topic_name);
  strcat(publish_topic,"/state");
  return publishToMQTT(state,publish_topic,false);
}

/*
 * Callback called on receiving a message. It posts the incoming message in the queue
 */
void OnDataSent(uint8_t *receiver_mac, uint8_t transmissionStatus) {
  if(transmissionStatus == 0) {
    DPRINTLN("Data sent successfully");
  } else {
    DPRINT("Error code: ");DPRINTLN(transmissionStatus);
  }
};

/*
 * Callback called on sending a message.
 */
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  espnow_message msg;
  memcpy(&msg, incomingData, sizeof(msg));
  DPRINTF("OnDataRecv:%lu,%d,%d,%d,%d,%f,%f,%f,%f,%s,%s\n",msg.message_id,msg.intvalue1,msg.intvalue2,msg.intvalue3,msg.intvalue4,msg.floatvalue1,msg.floatvalue2,msg.floatvalue3,msg.floatvalue4,msg.chardata1,msg.chardata2);
  
    if(!structQueue.isFull())
      structQueue.enqueue(msg);
    else
      DPRINTLN("Queue Full");
};

/*
 * creates data for health message and publishes it
 * takes bool param init , if true then publishes the startup message else publishes the health check message
 */
bool publishHealthMessage(bool init=false)
{
  char publish_topic[65] = "";
  String str_msg="";

  if(init)
  {
     // publish the init message
    StaticJsonDocument<MAX_MESSAGE_LEN> init_msg_json;//It is recommended to create a new obj than reuse the earlier one by ArduinoJson
    init_msg_json["version"] = compile_version;
    init_msg_json["tot_memKB"] = (float)ESP.getFlashChipSize() / 1024.0;
    init_msg_json["mac"] = WiFi.macAddress();
    init_msg_json["macAP"] = WiFi.softAPmacAddress();
    init_msg_json["wifiChannel"] = WiFi.channel();
    strcpy(publish_topic,MQTT_TOPIC);
    strcat(publish_topic,"/init");
    str_msg="";
    serializeJson(init_msg_json,str_msg);
    return publishToMQTT(str_msg.c_str(),publish_topic,true);
  }
  else
  {
    StaticJsonDocument<MAX_MESSAGE_LEN> msg_json;
    msg_json["uptime"] = millis()/1000; //publish uptime in seconds
    msg_json["mem_freeKB"] = serialized(String((float)ESP.getFreeHeap()/ 1024.0,0));//Ref:https://arduinojson.org/v6/how-to/configure-the-serialization-of-floats/
    msg_json["msg_count"] = message_count;
    msg_json["queue_len"] = structQueue.itemCount();
    float message_rate = (message_count - last_message_count)/(float)(HEALTH_INTERVAL/(60*1000));//rate calculated over one minute
    last_message_count = message_count;//reset the count
    msg_json["msg_rate"] = serialized(String(message_rate,1));//format with 1 decimal places, Ref:https://arduinojson.org/v6/how-to/configure-the-serialization-of-floats/

    // create a path for this specific device which is of the form MQTT_BASE_TOPIC/<master_id> , master_id is usually the mac address stripped off the colon eg. MQTT_BASE_TOPIC/2CF43220842D
    strcpy(publish_topic,MQTT_TOPIC);
    strcat(publish_topic,"/state");
    serializeJson(msg_json,str_msg);
    if(publishToMQTT(str_msg.c_str(),publish_topic,false))
    {
      // publish the wifi message , I am publishing this everytime because it also has rssi
      strIP_address = WiFi.localIP().toString();
      StaticJsonDocument<MAX_MESSAGE_LEN> wifi_msg_json;//It is recommended to create a new obj than reuse the earlier one by ArduinoJson
      wifi_msg_json["ip_address"] = strIP_address;
      wifi_msg_json["rssi"] = WiFi.RSSI();
      strcpy(publish_topic,MQTT_TOPIC);
      strcat(publish_topic,"/wifi");
      str_msg="";
      serializeJson(wifi_msg_json,str_msg);
      return publishToMQTT(str_msg.c_str(),publish_topic,true);
   }
  }
  return false;//control will never come here
}

void printInitInfo()
{
  DPRINTLN("Starting up as a ESPNow Gateway");
  #if USING(MOTION_SENSOR)
    DPRINTLN("Motion sensor ON");
  #else
    DPRINTLN("Motion sensor OFF");
  #endif
  #if USING(SECURITY)
    DPRINTLN("Security ON");
  #else
    DPRINTLN("Security OFF");
  #endif

}

/*
 * Initializes the ESP with espnow and WiFi client , OTA etc
 */
void setup() {
  // Initialize Serial Monitor , if we're usng pin 3 on ESP8266 for PIR then initialize Serial as Tx only , disable Rx
  #if USING(MOTION_SENSOR)
  if(PIR_PIN == 3)
    {DBEGIN(115200, SERIAL_8N1, SERIAL_TX_ONLY);}
  else
    {DBEGIN(115200);}
  #else
    {DBEGIN(115200);}
  #endif

  printInitInfo();

  // Set the device as a Station and Soft Access Point simultaneously
  WiFi.mode(WIFI_AP_STA); // This has to be WIFI_AP_STA and not WIFI_STA, I dont know why but if set to WIFI_STA then it can only receive broadcast messages
  // and stops receiving MAC specific messages.
  // Set a custom MAC address for the device. This is helpful in cases where you want to replace the actual ESP device in future
  // A custom MAC address will allow all sensors to continue working with the new device and you will not be required to update code on all devices
  uint8_t customMACAddress[] = DEVICE_MAC; // defined in Config.h
  if(wifi_set_macaddr(SOFTAP_IF, &customMACAddress[0]))
    { DPRINT("Successfully set a custom MAC address as:");
      DPRINTLN(WiFi.softAPmacAddress());
    }
    else
    {DPRINT("Failed to set custom MAC address:");}

  pinMode(STATUS_LED,OUTPUT);
  WiFi.config(ESP_IP_ADDRESS, default_gateway, subnet_mask);//from secrets.h
  String device_name = DEVICE_NAME;
  device_name.replace("_","-");//hostname dont allow underscores or spaces
  WiFi.hostname(device_name.c_str());// Set Hostname.


  // For Station Mode
  //wifi_set_macaddr(STATION_IF, &newMACAddress[0]);

  // Set device as a Wi-Fi Station
  WiFi.begin(ssid, password);
  DPRINTLN("Setting as a Wi-Fi Station..");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    DPRINT(".");
  }
  DPRINT("Station IP Address: ");
  DPRINTLN(WiFi.localIP());
  DPRINT("Wi-Fi Channel: ");
  DPRINTLN(WiFi.channel());
  WiFi.setAutoReconnect(true);

  if((WiFi.status() == WL_CONNECTED))
  {
    client.setServer(mqtt_broker, mqtt_port);// from secrets.h
  }
  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    DPRINTLN("Error initializing ESP-NOW");
    return;
  }
  
  esp_now_set_self_role(MY_ROLE);
  #if USING(SECURITY)
  // Setting the PMK key
  esp_now_set_kok(kok, KEY_LEN);
  byte channel = wifi_get_channel();
  // Add each controller who is expected to send a message to this gateway
  for(byte i = 0;i< sizeof(controller_mac)/6;i++)
  {
    esp_now_add_peer(controller_mac[i], RECEIVER_ROLE, channel, key, KEY_LEN);
    DPRINTFLN("Added controller :%02X:%02X:%02X:%02X:%02X:%02X",controller_mac[i][0],controller_mac[i][1],controller_mac[i][2],controller_mac[i][3],controller_mac[i][4],controller_mac[i][5] );
  }
   #endif

  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);
  DPRINT("WiFi address:");DPRINTLN(WiFi.macAddress());
  DPRINT("SoftAP address:");DPRINTLN(WiFi.softAPmacAddress());

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
  reconnectMQTT(); //connect to MQTT before publishing the startup message
  if(publishHealthMessage(true)) //publish the startup message
    initilised = true;
  #if USING(MOTION_SENSOR)
  if(!motion_sensor.begin(MOTION_SENSOR_NAME)) //initialize the motion sensor
    DPRINTLN("Failed to initialize motion sensor");
  #endif
  
}


/*
 * runs the loop to check for incoming messages in the queue, picks them up and posts them to MQTT
 */
void loop() {
  //check for MQTT connection
  if (!client.connected()) 
  {
    reconnectMQTT(); // Attempt to reconnect
  } else {
    client.loop(); // Client connected
  }
  MQTT_wd.update(client.connected()); //feed the watchdog by calling update
  
  #if USING(MOTION_SENSOR)
  short motion_state = motion_sensor.update();
  if(motion_state == 1)
  {
    DPRINTLN("Motion detected as ON");
    publishMotionMsgToMQTT(motion_sensor.getSensorName(),"on");
  }
  else if(motion_state == 2)
  {
    DPRINTLN("Motion detected as OFF");
    publishMotionMsgToMQTT(motion_sensor.getSensorName(),"off");
  }
  #endif

  ArduinoOTA.handle();
  
  if(client.connected())
  {
    if(!structQueue.isEmpty())
      if(!retry_message)
        currentMessage = structQueue.dequeue();
      //else last message content is still there is currentMessage
    
    if(currentMessage != emptyMessage)
    {
      if(publishToMQTT(currentMessage))
      {
        message_count++;
        currentMessage = emptyMessage;
      }
      else
      {
        retry_message = true;
      }//else the same message will be retried the next time
    }
  }

  // try to publish health message irrespective of the state of espnow messages
  if(millis() - last_time > HEALTH_INTERVAL)
  {
    // It might be possible when the ESP comes up MQTT is down, in that case an init message will not get published in setup()
    // The statement below will check and publish the same , only once
    if(!initilised)
    {
      if(publishHealthMessage(true))
        initilised = true;
    }
    //Now publish the health message
    publishHealthMessage();
    last_time = millis(); // This is reset irrespective of a successful publish else the main loop will continously try to publish this message
  }
  statusLED.loop();
}
