/*
 *secrets.h - file to store secret strings like WiFi credentials ets
 * Include this file into your programs and refer the variables here to pick up thier values
 * This file should not be checked into git. Advantage of this file is that you dont have to check in your secret info on git
*/

#ifndef SECRETS_H
#define SECRETS_H

#define primary_ssid "YOUR SSID"
#define primary_ssid_pswd "YOUR PSWD"
#define gf_ssid "YOUR SSID2"
#define gf_ssid_pswd "YOUR PSWD2"
#define default_gateway IPAddress(192,168,1,1)
#define default_dns IPAddress(192,168,1,1)
#define subnet_mask IPAddress(255,255,255,0)
#define mqtt_broker IPAddress(192,168,1,XXX)
#define mqtt_port 1883
#define mqtt_uname "XXX"
#define mqtt_pswd "XXXX"

#endif
