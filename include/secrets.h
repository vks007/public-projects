/*
 *secrets.h - file to store secret strings like WiFi credentials ets
 * Include this file into your programs and refer the variables here to pick up thier values
 * This file should not be checked into git. Advantage of this file is that you dont have to check in your secret info on git
*/

#ifndef SECRETS_H
#define SECRETS_H
#include "static_ipaddress.h"

#define primary_ssid "YOUR SSID"
#define primary_ssid_pswd "YOUR PSWD"
#define gf_ssid "YOUR SSID2"
#define gf_ssid_pswd "YOUR PSWD2"
#define default_gateway IPAddress(192,168,1,1)
#define default_dns IPAddress(192,168,1,1)
#define subnet_mask IPAddress(255,255,255,0)
#define mqtt_broker IPAddress(192,168,1,10)
#define mqtt_port 1883
#define mqtt_uname "XXX"
#define mqtt_pswd "XXXX"
#define ota_pswd "XXXX"
#define api_pswd "XXXX"
#define captive_portal_pswd "XXXX"
#define primary_mesh_name "XXXX"
#define primary_mesh_pswd "XXXX"
#define primary_mesh_port 4326
#define PMK_KEY_STR {0x11, 0x22, 0x33, 0x44, 0x44, 0x56, 0x77, 0x88, 0xFF, 0x00, 0x01, 0x12, 0x32, 0x44, 0x45, 0x66} // define any random key
#define LMK_KEY_STR {0x11, 0x22, 0x33, 0x44, 0x42, 0x56, 0x27, 0x88, 0xF2, 0x00, 0x01, 0x13, 0x32, 0x22, 0x45, 0x67} // define any random key
#define CONTROLLERS { \
	{0x4C, 0x64, 0x33, 0x22, 0x44, 0x2D} ,\
	{0x3C, 0x2D, 0x12, 0x73, 0x41, 0x88}\
	}
#define GATEWAY_FF_AP_MAC {0x22, 0x41, 0x44, 0x55, 0xA2, 0x8E} //- This is the SoftAP MAC addr of the ESP01 FF Gateway
#define GATEWAY_GF_AP_MAC {0xEE, 0xFF, 0x12, 0x13, 0xF7, 0x5D} //- This is the SoftAP MAC addr of the ESP01 GF Gateway
#define BROADCAST_MAC  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}// broadcast gateway - to everyone

#endif


