#ifndef HTTP_SERVICES
#define HTTP_SERVICES
#include "Arduino.h"
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>

static WiFiClient espClient;
static HTTPClient http;

class HttpServices

{
public:
    const String get(const String &url, const String &token);
    const String post(const String &url, const String &payload);
    const String post(const String &url, const String &payload, const String &token);
    //    const String signinWithemailAndPassword(const String &url ,const String &payload ,const String &token);

private:
    void _begin();
    void end();
};

#endif // !HTTP_SERVICES
