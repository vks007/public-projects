/*
 *espwatchdog.h - file to provide auto ESP restart functionality for microcontrollers based on conditions defined in the calling code
 * 
 * 
*/

#ifndef ESPWATCHDOG_H
#define ESPWATCHDOG_H
#define DEFAULT_TIMEOUT 600 // in seconds
// At present this file will only work for ESP8266 uC. Will enhance it for ESP32 too later
#ifdef ESP8266
#include <Arduino.h>
//#include <ESP8266WiFi.h>

class watchDog
{
    public:
    watchDog(int timeout= DEFAULT_TIMEOUT)
    {
        _timeout = timeout*1000;
    }
    void setTimeout(int timeout)
    { _timeout = timeout*1000;}
    
    int getTimeout()
    { return _timeout/1000;}
    
    void update(bool monitor_flag)
    {
        _monitor_flag = monitor_flag;
        if(monitor_flag)
        {
            _last_timestamp = millis();
        }
        else
        {
            if( millis() - _last_timestamp > _timeout)
            {
                ESP.restart();
            }
        }
    }
    private:
    int _timeout = 0;
    bool _monitor_flag = false;
    long _last_timestamp = 0;
};
#endif //ESP8266

#endif