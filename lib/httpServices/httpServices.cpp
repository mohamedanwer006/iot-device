#include "httpServices.h"

const String HttpServices::get(const String &url, const String &token)
{
    const String *payload ;
    int httpCode;
    Serial.print("[HTTP] begin...\n");
    http.begin(espClient, url);
    String authtoken = "Bearer " + token;
    Serial.print("Authorization:");
    Serial.println(authtoken);
    Serial.print("[HTTP] begin...\n");
    http.addHeader("Authorization", authtoken);
    Serial.print("[HTTP] GET...\n");
    // start connection and send HTTP
    httpCode = http.GET();

    // httpCode will be negative on error
    if (httpCode > 0)
    {
        // HTTP has been send and Server response header has been handled
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);
        // file found at server
        if (httpCode == HTTP_CODE_OK)
        {
            payload = &http.getString();
            Serial.println("received payload:\n<<");
            Serial.println(*payload);
            Serial.println(">>");
        }
    }
    else
    {
        Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
    return *payload;
}

const String HttpServices::post(const String &url,const String &payload)
{
    const String *responceBody ;
    int httpCode;
    Serial.print("[HTTP] begin...\n");
    http.begin(espClient, url); // HTTP
    http.addHeader("Content-Type", "application/json");
    // http.addHeader("Authorization", "Bearer ");
    Serial.print("[HTTP] POST...\n");
    // start connection and send HTTP header and body
     httpCode = http.POST(payload);
    // httpCode will be negative on error
    if (httpCode > 0)
    {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] POST... code: %d\n", httpCode);
        // file found at server
        if (httpCode == HTTP_CODE_OK)
        {
            responceBody = &http.getString();
            Serial.println("received payload:\n<<");
            Serial.println(">>");
        }
    }
    else
    {
        Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
    return *responceBody;
}const String HttpServices::post(const String &url,const String &payload , const String &token)
{
    const String *responceBody ;
    int httpCode;
    Serial.print("[HTTP] begin...\n");
    String authtoken = "Bearer " + token;
    http.begin(espClient, url); // HTTP
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", authtoken);
       
    Serial.print("[HTTP] POST...\n");
    // start connection and send HTTP header and body
     httpCode = http.POST(payload);
    // httpCode will be negative on error
    if (httpCode > 0)
    {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] POST... code: %d\n", httpCode);
        // file found at server
        if (httpCode == HTTP_CODE_OK)
        {
            responceBody = &http.getString();
            Serial.println("received payload:\n<<");
            Serial.println(">>");
        }
    }
    else
    {
        Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
    return *responceBody;
}