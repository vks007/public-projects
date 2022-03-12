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
#define mqtt_broker IPAddress(192,168,1,XXX)
#define mqtt_port 1883
#define mqtt_uname "XXX"
#define mqtt_pswd "XXXX"
#define ota_pswd "XXXX"
#define api_pswd "XXXX"
#define captive_portal_pswd "XXXX"
#define primary_mesh_name "XXXX"
#define primary_mesh_pswd "XXXX"
#define primary_mesh_port 4326
#define PMK_KEY_STR {0x11, 0x22, 0x322, 0x43, 0x54, 0x66, 0x77, 0x88, 0xF9, 0x11, 0xA1, 0x22, 0x32, 0x44, 0x45, 0x76}
#define LMK_KEY_STR {0x33, 0x43, 0x33, 0x14, 0x22, 0x14, 0x33, 0xF4, 0x13, 0x44, 0xA3, 0x14, 0x73, 0x44, 0x3C, 0x57}
#define CONTROLLERS { \
	{0x4C, 0x64, 0x33, 0x22, 0x44, 0x2D} ,\
	{0x3C, 0x2D, 0x12, 0x73, 0x41, 0x88}\
	}
#define GATEWAY_FF_MAC {0x22, 0x41, 0x44, 0x55, 0xA2, 0x8E} //- This is the SoftAP MAC addr of the ESP01 FF Gateway
#define GATEWAY_GF_MAC {0xEE, 0xFF, 0x12, 0x13, 0xF7, 0x5D} //- This is the SoftAP MAC addr of the ESP01 GF Gateway
#define BROADCAST_MAC  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}// broadcast gateway - to everyone

#endif


