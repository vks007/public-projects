/*
 *DebugUtils.h - Simple debugging utilities. Include the file "Debugutils.h in your program.
 * To turn DEBUG ON & OFF put the following statement in your program , this statement should be before the include statement for Debugutils.h
 #define DEBUG (1) // Turn DEBUG ON
 #define DEBUG (0) // Turn DEBUG OFF
 #include "Debugutils.h"
*/

#ifndef DEBUGUTILS_H
#define DEBUGUTILS_H

#if (DEBUG)
  #define DPRINT(...) Serial.print(__VA_ARGS__)
  #define DPRINTLN(...) Serial.println(__VA_ARGS__)
  #define DBEGIN(...) Serial.begin(__VA_ARGS__)
  #define DPRINTF(...) Serial.printf(__VA_ARGS__)
  #define DPRINTFLN(...) Serial.printf(__VA_ARGS__);Serial.printf("\n")
  #define DFLUSH()		Serial.flush()
  #define DEND()		Serial.end()
#else
  #define DPRINT(...)
  #define DPRINTLN(...)
  #define DBEGIN(...)
  #define DPRINTF(...)
  #define DPRINTFLN(...)
  #define DFLUSH()
  #define DEND()
#endif

#endif
