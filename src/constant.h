#ifndef CONSTANT_H
#define CONSTANT_H

#include "Arduino.h"

#define CONFIG_PATH "/config.json"
#define DEVICE_PATH "/device.json"

#define OUT1 D1


// access point
// static const char *ssid = "ACDevice";
// static const char *wpa2 = "MD123456789";

#define AP_SSID "ACDevice"
#define AP_PASSWORD "MD123456789"
// broker


static const char *username = "zwccckwo";
static const char *password = "lm8yAHH_5KWj";
static const char *mqtt_server = "m16.cloudmqtt.com";

#define USERNAME "zwccckwo"
#define PASSWORD "lm8yAHH_5KWj"
#define MQTT_SERVER "m16.cloudmqtt.com"

//api  
#define AUTH_URL "http://api-engine-v1.herokuapp.com/auth/local"
#define DEVICE_URL "http://api-engine-v1.herokuapp.com/api/v1/devices/"



#endif // !CONSTANT_H


