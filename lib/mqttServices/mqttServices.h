#ifndef Mqtt_SERVICES
#define Mqtt_SERVICES

#include "Arduino.h"
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include <PubSubClient.h>
#include "ArduinoJson.h"
#include "/media/mohamed/01D57712DBDF0340/arduino/node_ap_server/src/constant.h"

static WiFiClient espClient;
static PubSubClient client(espClient);

class MqttServices
{
    static void callback(char *topic, byte *payload, unsigned int length)
    {
        const size_t capacityForMyData = JSON_OBJECT_SIZE(1) + 10;
        DynamicJsonDocument mydataDoc(capacityForMyData);

        // const char* json = "{\"value\":\"on\"}";
        // const char* value = doc["value"]; // "on"
        Serial.print("Message arrived [");
        Serial.print(topic);
        Serial.print("] ");
        String data = "";

        for (unsigned int i = 0; i < length; i++)
        {
            data = data + (char)payload[i];
            // Serial.print((char)payload[i]);
        }
        Serial.println(data);
        deserializeJson(mydataDoc, data);
        //
        String value = mydataDoc["value"]; // "on" or "off"
        Serial.print("value: ");
        Serial.println(value);
        if (value == "on")
        {
            digitalWrite(LED_BUILTIN, LOW); // Turn the LED on (Note that LOW is the voltage level
            digitalWrite(OUT1, HIGH);
        }
        else
        {
            digitalWrite(LED_BUILTIN, HIGH); // Turn the LED off by making the voltage HIGH
            digitalWrite(OUT1, LOW);         // Turn the LED off by making the voltage HIGH
        }
    }

public:
    MqttServices(String broker, String username, String password)
    {
        _broker = broker;
        _username = username;
        _password = password;
    };

    void connect();
    bool connected();
    void loop(String deviceId);

private:
    void reconnect(String deviceId);
    String _broker;
    String _username;
    String _password;
};

#endif // !Mqtt_SERVICES
