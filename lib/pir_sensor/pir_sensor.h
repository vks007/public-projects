/*
 * References to figure out how to call class instance method as ISR
 * Final place from where the code worked : https://stackoverflow.com/questions/60648871/c-callback-for-c-function
 * Could not make sense of this but maybe useful : https://www.onetransistor.eu/2019/05/arduino-class-interrupts-and-callbacks.html
 * 
 */

#ifndef PIR_SENSOR_H
#define PIR_SENSOR_H

#include <Arduino.h>
#include <FunctionalInterrupt.h>

class pir_sensor {
private:
    pir_sensor();//disable constructor without pin
    char _sensor_name[25];//name of the sensor
    int _signal_pin;
    bool volatile _motion_on; // flag to capture when motion signal goes HIGH on the signal pin , remains true for the entire duration of when signal remains HIGH
    bool volatile _motion_triggered; // flag to capture when motion turns ON , sets the _motion_state flag and then resets to false
    bool _motion_state; // flag to capture when signal pin goes HIGH, is set to false once the caller calls update() till motion turns OFF and again ON
    unsigned long volatile _motion_timer; // keeps track of the no of millis elapsed since motion was triggered
    unsigned int _motion_duration = 5000; // duration of time for which motion should not be re-triggered if it is already ON , defaults to 5 sec
    bool _sensor_ready = false; // true if sensor is inialized properly else false
public:
    String sensor_name;
    pir_sensor(byte pin): _signal_pin(pin) {}
    pir_sensor(byte pin,uint16_t duration): _signal_pin(pin),_motion_duration(1000 *duration) {}
    bool begin(const char name[25]);
    ~pir_sensor()
    {
      detachInterrupt(_signal_pin);
    }
    void IRAM_ATTR pir_callback();
    short update();
    void setMotionDuration(uint16_t duration);
    const char* getSensorName();
};

#endif
