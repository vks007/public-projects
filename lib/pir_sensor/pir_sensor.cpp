/*
 * PIR sensor library. This library operates on a PIR sensor and uses interrupts to sense motion
 * - create an object of the pir_sensor class passing in the pin no on which the PIR signal pin is connected.
 * - Then call begin() method to initialize the library
 * - Call update() method in loop. If update returns true, motion is triggered, else it returns false.
 * Features:
 *  - Holds motion ON for a certain time period irrespective of repeated motion signals within that timeframe. This time can be configured via setMotionDuration()
 *  - update returns true only the first time after motion was triggered, returns false thereafter until motion turns OFF and then ON as set by etMotionDuration()
 *  - checks if the pin passed for PIR signal supports interrupts, if not begin() returns false.
 *  - Checks if the PIR pin passed is either of Rx or Tx , if so changes the function of the pin to GPIO before setting it as INPUT
 */
 
#include "pir_sensor.h"
// //#include "Debugutils.h"

// TO DO : ************* I am not able to include the file Debugutils.h here to print serial debug statements , somehow the #define for SERIAL_DEBUG isnt defined
// when the file Debugutils gets included here
// It results in an compile error , will look into this later

// begin() is to be called the first after creation of the object of pir_sensor. It initializes the pin & interrupt
// Returns true if successful , returns false if the pin is not an interruptable pin
bool pir_sensor::begin(const char name[25])
{
  if(name != NULL) //assign a name to sensor if it was not passed
    strcpy(_sensor_name,name);
  else
    strcpy(_sensor_name,"motion_sensor1");

  if (_signal_pin != NOT_AN_INTERRUPT)
  {
    #ifdef ESP8266
    #define TX_PIN 1
    #define RX_PIN 3
    if(_signal_pin == RX_PIN || _signal_pin == TX_PIN)
        {pinMode(_signal_pin, FUNCTION_3);}
    #endif

    pinMode(_signal_pin, INPUT);
    attachInterrupt(digitalPinToInterrupt(_signal_pin), std::bind(&pir_sensor::pir_callback,this), RISING);
    _sensor_ready = true;
    return true;
  }
  //DPRINTLN("Sensor initalized failed, signal pin is not interruptable");
  return false;
}

// callback to be called when the interrupt triggers on the signal pin, the type of interrupt is RISING
void IRAM_ATTR pir_sensor::pir_callback()
{
    _motion_on = true;
    _motion_triggered = true;
    _motion_timer = millis();
}
// updates the state of motion of the sensor. Returns TRUE if motion has triggered afresh since update was last called 
// returns 0 if motion has not triggered since it was last called.
// Returns 1 if motion has been triggered since this method was last called
// Returns 2 if motion has turned OFF after it was ON previously
short pir_sensor::update()
{
    if(!_sensor_ready)
    {
      //DPRINTLN("Sensor not initalized, call begin()");
      return 0;
    }
    if(_motion_triggered)
    {
        _motion_triggered = false;
        _motion_state = true;
        //DPRINTLN("motion turned ON");
    }
    if(((millis() - _motion_timer) > _motion_duration) && _motion_on )
    {
        if(!digitalRead(_signal_pin))
        {
          _motion_on = false;
          _motion_triggered = false;
          _motion_state = false;
          //DPRINTLN("motion turned OFF");
          return 2;
        }
        else
        {
          _motion_timer = millis();
          //DPRINTLN("motion time renewed");
        }
    }
    if(_motion_state)
    {
      _motion_state = false;
      return 1;
    }
    return 0;
   
}
// sets the duration in seconds for which motion is to be held ON once triggered 
void pir_sensor::setMotionDuration(uint16_t duration)
    {_motion_duration = duration* 1000;}

const char* pir_sensor::getSensorName()
{
  return _sensor_name;
}
