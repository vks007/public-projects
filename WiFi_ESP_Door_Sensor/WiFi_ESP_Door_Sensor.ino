/*
 * NOTE IMPORTANT : BUILD SETTINGS : File Size: 512K no SPIFFS and no OTA for ESP01 or choose similar size for ESP12
 * 2.1.1 - moved wifi info to a separate topic , moved other params like ESP type, ssid etc into DoorConfig.h , included compile version
 * 2.0.0 - removed usage of SPIFFS, instead moved to hardcoded values from header file. removed usage of WiFimanager
 * 1.4.1 - removed config of individual topics , rather only take main topic and hard code subsequent paths. Also included more messages to be logged on MQ Broker , like TESTING mode etc
 * 1.4.0 - makes user configuration configurable via a config file (as in Version 1.0)
 * 1.3 - Implements receiving message type from ATTiny on Rx/Tx pins via a 2 bit code
 * Works with ATTiny door sensor V 1.3 onwards
 * 
 * TO DO List
 * Introduce AsyncMQTTclient library to publish messages with QoS 1 , you get back an ack id which confirms the message was published , if not , you get back 0
 * If you get back 0 , you have to publish the message again (not sure if you mark the message try as duplicate though
 * Introduce a status LED on GPIO2 to blink thrice and show that a message publish was successful. This can be done as CONFIG functionality is not needed. Even if you need it, you can set
 * the GPIO2 as INPUT on startup (as you do now) and then change it to OUTPUT 
 * 
 * DONE List
 * Remove usage of FS/SPIFFS in this, take values form secrets file and build a binary specific for each module as is the case with ESPHome
 * Mark the messages & topics as retained (flag passed during publish)
 * Remove the usage of WiFiManager - i think this is not needed at least for my case , why complicate things as if ESP ever gets stuck in a loop without Wifi, it will drain the battery
 * the battery status goes to unknown after some time while I would want it to stay at the last known value - i think this might get solved by retained flag
 * Solve the bug where I dont see the topic on MQTT in spite the fact that this program publishes it - this has been taken care of by retained messages. 
*/
#include <PubSubClient.h>
#include <ESP8266WiFi.h>

// Uncomment one of the below define to compile the program for that device
#define MAIN_DOOR
//#define TERRACE_DOOR
//#define BALCONY_DOOR

//#define TESTING_MODE //used to prevent using Rx & Tx as input pins , rather use them as normal serial pins for debugging , comment this out during normal operation
#define DEBUG (0) //BEAWARE that this statement should be before #include "Debugutils.h" else the macros wont work as they are based on this #define
#include "Debugutils.h" //This file is located in the Sketches\libraries\DebugUtils folder

#include "secrets.h" 
#include "DoorConfig.h"

#define VERSION "2.1.1"
const char compile_version[] = VERSION " " __DATE__ " " __TIME__; //note, the 3 strings adjacent to each other become pasted together as one long string
//For some reason I get an error that compile_version is not defined in this scope if I use version.h where the above statement is written, so writing it inline instead.

//Types of messages decoded via the signal pins
#define SENSOR_NONE 0
#define SENSOR_WAKEUP 1
#define SENSOR_OPEN 2
#define SENSOR_CLOSED 3
#define MAX_MQTT_CONNECT_RETRY 4 //max no of retries to connect to MQTT server

#define MSG_ON "on" //payload for ON
#define MSG_OFF "off"//payload for OFF

ADC_MODE(ADC_VCC);//connects the internal ADC to VCC pin and enables measuring Vcc

WiFiClient espClient;
PubSubClient client(espClient);

void setup() 
{
  DBEGIN(115200);
  pinMode(HOLD_PIN, OUTPUT);
  digitalWrite(HOLD_PIN, LOW);  // sets GPIO0 to high (this holds CH_PD high even if the input signal goes LOW)

  short CURR_MSG = SENSOR_NONE; //This stores the message type deciphered from the states of the signal pin
  short PREV_MSG = SENSOR_NONE; //This stores the message type deciphered from the states of the signal pin

  DPRINT("State of GPIO0:");DPRINTLN(digitalRead(HOLD_PIN));
  #ifndef TESTING_MODE
  if(SIGNAL_PIN == 1 || SIGNAL_PIN == 3)
    pinMode(SIGNAL_PIN, FUNCTION_3);//Because we're using Rx & Tx as inputs here, we have to set the input type
  pinMode(SIGNAL_PIN, INPUT_PULLUP);     
  #endif
  
  DPRINTLN("");
  DPRINTLN("Version:" + String(compile_version));
  DPRINTLN("Going to setup wifi");
  setupWiFi();
  
  if((WiFi.status() == WL_CONNECTED))
  {
    DPRINT("WiFi connected, IP Address:");
    DPRINTLN(WiFi.localIP());
    
    client.setServer(mqtt_server, mqtt_port_no);// from secrets.h

    //TO DO : you can read the input values in a single statement directly from registers and then compare using a mask
    // TO DO : Shift the reading of pins to before reading config so that even if ATTiny removes the signal, ESP can still take its own time in publishing the message
    //Read the type of message we've got from the ATiny
    #ifdef TESTING_MODE
      CURR_MSG = SENSOR_WAKEUP;
    #else
    if((digitalRead(SIGNAL_PIN) == LOW))
      CURR_MSG = SENSOR_CLOSED;
    else 
      CURR_MSG = SENSOR_OPEN;
    #endif

    while(CURR_MSG != PREV_MSG)
    {
      if(CURR_MSG != SENSOR_NONE)
      {
        publishMessage(CURR_MSG);
        //Allow a delay to let MQTT publish the message as the publish method is asyncronous, If I dont put this, the ESP powers down before the msg is published
        delay(1000);
      }
      
      //Read the sensor again to see if it has changed from last time, if yes then repeat the loop to publish this message
      PREV_MSG = CURR_MSG;
      if((digitalRead(SIGNAL_PIN) == LOW))
        CURR_MSG = SENSOR_CLOSED;
      else
        CURR_MSG = SENSOR_OPEN;
    }
  }
  else
  {
    DPRINTLN("Could not connect to WiFi. Check the WiFi credentials");
  }

  //Power down the ESP 
  DPRINTLN("powering down");
  digitalWrite(HOLD_PIN, HIGH);  // set HOLD PIN HIGH to cutt off power
}

void setupWiFi() 
{
  WiFi.config(ESP_IP_ADDRESS, default_gateway, subnet_mask);//from secrets.h
  WiFi.hostname(DEVICE_NAME);// Set Hostname.
  DPRINTLN(".....");
  
  WiFi.mode(WIFI_STA); // Force to station mode because if device was switched off while in access point mode it will start up next time in access point mode.
  WiFi.begin(WiFi_SSID,WiFi_SSID_PSWD);
  for(short i=0;i<3000;i++) //break after 30 sec 3000*10 msec
  {
    if(WiFi.status() != WL_CONNECTED)
    {
      delay(10);//Dont increase this delay, I set it to 500 and it takes a very long time to connect, I think this blocks the execution
    }
    else
    {
      DPRINT("WiFi connected, IP Address:");
      DPRINTLN(WiFi.localIP());
      break;
    }
  }
}

void publishMessage(short msg_type) {
  // Loop until we're reconnected
  char i = 0;
  while (!client.connected()) 
  {
    i++;
    DPRINT("Attempting MQTT connection...");
    char publish_topic[65] = ""; //variable accomodates 50 characters of main topic + 15 char of sub topic
    strcpy(publish_topic,MQTT_TOPIC);
    strcat(publish_topic,"/LWT"); 
    // Attempt to connect
    if (client.connect(DEVICE_NAME,mqtt_user,mqtt_password,publish_topic,0,true,"offline")) {
      DPRINTLN("connected");
      strcpy(publish_topic,MQTT_TOPIC);
      strcat(publish_topic,"/status");
      int msgLen = String(MSG_ON).length();
      if(msg_type == SENSOR_OPEN)
        client.publish(publish_topic,MSG_ON,true);
      else if(msg_type == SENSOR_CLOSED)
        client.publish(publish_topic, MSG_OFF,true);
      
      //Given the sensor will only be online for a few secs, it doesnt make sense to publish availability message
      //strcpy(publish_topic,MQTT_TOPIC);
      //strcat(publish_topic,"/availability"); 
      //client.publish(publish_topic, "online",true);

      //measure batery voltage and publish that too
      int battery_Voltage = ESP.getVcc();
      char batt_volt[6];
      itoa(battery_Voltage, batt_volt, 10);

      short testing_mode = 0;
      #ifdef TESTING_MODE
        testing_mode = 1;
      #endif

      //I follow google style naming convention for json which is camelCase
      //publish the wifi message
      String state_json = String("{\"ip_address\":\"") + WiFi.localIP().toString() + String("\",\"mac\":\"") + WiFi.macAddress() + String("\"}");
      strcpy(publish_topic,MQTT_TOPIC);
      strcat(publish_topic,"/wifi"); 
      client.publish(publish_topic, state_json.c_str(),true);
      
      //publish the state message
      state_json = String("{\"upTime\":") + millis() + String(",\"vcc\":") + batt_volt + String(",\"version\":\"") + compile_version + String("\",\"testingMode\":") + testing_mode + String("}");
      strcpy(publish_topic,MQTT_TOPIC);
      strcat(publish_topic,"/state"); 
      client.publish(publish_topic, state_json.c_str(),true);

      DPRINTLN("published messages");
    } 
    else 
    {
      DPRINT("failed, rc=");
      DPRINT(client.state());
      DPRINTLN(" try again in 1 second");
      delay(1000);
    }
    if(i >= MAX_MQTT_CONNECT_RETRY)
      break;
  }
}

void loop() 
{
  //Nothing to do here as the setup does all the work and then powers down the ESP 
  DPRINTLN("in loop, we should have never reached here !!!!");
  delay(2000);
}
