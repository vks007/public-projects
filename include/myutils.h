/*
 *myutils.h - Simple utilities commonly used in your programs
*/

#ifndef MYUTILS_H
#define MYUTILS_H


/*
 * This function provides a safe way to introduce delay by calling yield() every 10ms
 * Using this function will eliminate the chance of triggering a WDT
 * It seems this function may not be required in most cases, read here : https://www.sigmdel.ca/michel/program/esp8266/arduino/watchdogs_en.html
 */
void safedelay(unsigned int delay_time) {
  short n = delay_time / 10;//gives the no of times to loop with 10ms delay
  short remainder = delay_time % 10; //gives the remainder to give additional delay after looping n times
  for(int i = 0; i < n; i++) {
    delay(10);
    yield();
  }
  delay(remainder);
}


/*
 * Converts an IPAddress type to a string
 */
String IpAddress2String(const IPAddress& ipAddress)
{
  return String(ipAddress[0]) + String(".") +\
  String(ipAddress[1]) + String(".") +\
  String(ipAddress[2]) + String(".") +\
  String(ipAddress[3])  ; 
}


template <class X, class M, class N, class O, class Q>
X map_Generic(X x, M in_min, N in_max, O out_min, Q out_max){
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


/*
 * Converts milli seconds into a readable short string in the format days hour:min:sec ago. example : 5 days 04:09:16 ago
 */
String getReadableTime(unsigned long millis) {
  String readableTime = "";
  unsigned long seconds;
  unsigned long minutes;
  unsigned long hours;
  unsigned long days;

  seconds = millis / 1000;
  minutes = seconds / 60;
  hours = minutes / 60;
  days = hours / 24;
  millis %= 1000;
  seconds %= 60;
  minutes %= 60;
  hours %= 24;

  readableTime = String(days) + " ";
  
  if (hours < 10) {
    readableTime += "0";
  }
  readableTime += String(hours) + ":";

  if (minutes < 10) {
    readableTime += "0";
  }
  readableTime += String(minutes) + ":";

  if (seconds < 10) {
    readableTime += "0";
  }
  readableTime += String(seconds) + " ago";
  return readableTime;
}

#if defined(ESP8266)
uint32_t RTCmillis() {
  // system_get_rtc_time() is in us (but very inaccurate anyway)
  return (system_get_rtc_time() * (system_rtc_clock_cali_proc() >> 12)) / 1000;
}

// Write 16 bit int to RTC memory with checksum, return true if verified OK
// Slot 0-127
// (C) Turo Heikkinen 2019 , Source: https://hackaday.io/project/24993/instructions
bool writeRtcMem(uint16_t *inVal, uint8_t slot = 0) {
  uint32_t valToStore = *inVal | ((*inVal ^ 0xffff) << 16); //Add checksum
  uint32_t valFromMemory;
  if (ESP.rtcUserMemoryWrite(slot, &valToStore, sizeof(valToStore)) &&
      ESP.rtcUserMemoryRead(slot, &valFromMemory, sizeof(valFromMemory)) &&
      valToStore == valFromMemory) {
        return true;
  }
  return false;
}

// Read 16 bit int from RTC memory, return true if checksum OK
// Only overwrite variable, if checksum OK
// Slot 0-127
// (C) Turo Heikkinen 2019 , Source: https://hackaday.io/project/24993/instructions
bool readRtcMem(uint16_t *inVal, uint8_t slot = 0) {
  uint32_t valFromMemory;
  if (ESP.rtcUserMemoryRead(slot, &valFromMemory, sizeof(valFromMemory)) &&
      ((valFromMemory >> 16) ^ (valFromMemory & 0xffff)) == 0xffff) {
        *inVal = valFromMemory;
        return true;
  }
  return false;
}
#endif

/*
This short article presents trivial but fast and memory efficient function to compare two strings. 
Function returns true on success (strings are the same), else false
Source: https://blog.podkalicki.com/fast-string-comparison-for-microcontrollers/
*/
static bool xstrcmp(const char *s1, const char *s2)
{
    while(*s1 != '\0' && *s1 == *s2) {s1++; s2++;}
    return((*s1 - *s2) == 0);
}


#endif
