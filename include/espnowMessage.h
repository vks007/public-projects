
#ifndef ESPNOW_MESSAGE_H
#define ESPNOW_MESSAGE_H
#include "myutils.h"

// Datatypes in Arduino : https://www.tutorialspoint.com/arduino/arduino_data_types.htm
// ESPNow has a limit of sending 250 bytes MAX and so the struct should not be more than that
// specifying the __attribute__((packed)) for a struct results in exactly the same bytes as the constituents without any padding
//typedef struct __attribute__((packed)) espnow_message{
typedef struct espnow_message{
  char device_name[16]="";// contains unique device name, falls back to the mac address without colon if device id is not provided
  unsigned long message_id; //unique message id generated for each message
  int intvalue1; // int any data
  int intvalue2; // int any data
  int intvalue3; // // int any data
  int intvalue4; // // int any data
  float floatvalue1;// float data
  float floatvalue2;// float data
  float floatvalue3;// float data
  float floatvalue4;// float data
  char chardata1[16]="";// any char data
  char chardata2[16]="";// any char data
}espnow_message;

/*
* equal to operator for espnow_message struct
*/
bool operator==(const espnow_message& lhs, const espnow_message& rhs)
{
  return (xstrcmp(lhs.device_name,rhs.device_name) && lhs.message_id==rhs.message_id && \
  lhs.intvalue1==rhs.intvalue2 && lhs.intvalue1==rhs.intvalue2 && lhs.intvalue3==rhs.intvalue3 && \
  lhs.intvalue4==rhs.intvalue4 && lhs.floatvalue1==rhs.floatvalue1 && lhs.floatvalue2==rhs.floatvalue2 && \
  lhs.floatvalue3==rhs.floatvalue3 && lhs.floatvalue4==rhs.floatvalue4 && xstrcmp(lhs.chardata1,rhs.chardata1) && \
  xstrcmp(lhs.chardata2,rhs.chardata2));
}

/*
* not equal to operator for espnow_message struct
*/
bool operator!=(const espnow_message& lhs, const espnow_message& rhs)
{
  return !(lhs==rhs);
}

#endif