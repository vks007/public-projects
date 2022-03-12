/*
This file provides the version no of the firmware file in the following format : 1.0.1 Jul 20 2020 19:34:56
Define VERSION in your file and use the variable compile_version in your program
*/

#ifndef VERSION_H
#define VERSION_H

//put the below statement in your c file. 
//#define VERSION "1.0.1"

const char compile_version[] = VERSION " " __DATE__ " " __TIME__; //note, the 3 strings adjacent to each other become pasted together as one long string
/*
 * The C and C++ standards mandate that the compilers provide two magic macros that resolve to string literals:  
 * __DATE__ gives the time the file was compiled, and __TIME__ gives the current time of day
 * The compiler also has a _FILE_ macro that can print the sketch source filename
 */

#endif