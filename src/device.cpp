#include "device.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "httpServices.h"
#include <ArduinoJson.h>
#include "constant.h"

HttpServices httpServices;

void Device::init(){

    delay(200);
    pinMode(OUT1, OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(OUT1, LOW);
    // Turn the LED_BUILTIN off by making the voltage HIGH
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.begin(115200);
    Serial.println();
    // initialize file system
    fsServices.init();
}

void Device::run(){}
void Device::restrat(){}
//ToDo:
void Device::reset(){}


void Device::_signInWithEmailAndPassword(){
     const size_t cap = JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(5) + 600;
  DynamicJsonDocument res(cap);

  // Read user email pass from config file
  String json = fsServices.readFile(CONFIG_PATH);

  const size_t capacity1 = JSON_OBJECT_SIZE(4) + 400;
  DynamicJsonDocument doc(capacity1);
  DeserializationError error = deserializeJson(doc, json);
  if (error)
  {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
  }

  String email = doc["email"];       
  String password = doc["password"];

  const size_t capacity2 = JSON_OBJECT_SIZE(2)+50;
  DynamicJsonDocument docs(capacity2);

  docs["email"] = email;
  docs["password"] = password;

  String body;
  serializeJson(docs, body);
  Serial.println(body);
  deserializeJson(res, httpServices.post(AUTH_URL,body));

  String token = res["token"];
  String authtoken = "Bearer " + token;
  
  Serial.print("Authorization:");
  Serial.println(authtoken);
  Serial.print("[HTTP] begin...\n");

  const size_t deviceCapacity = JSON_OBJECT_SIZE(12) + 400;
  DynamicJsonDocument deviceData(deviceCapacity);
  const String deviceBody = "{\"name\":\"new device\"}";
  const String responseBody= httpServices.post(DEVICE_URL,deviceBody,authtoken);
  deserializeJson(deviceData,responseBody );
  String _id = deviceData["_id"];
  Serial.print("device_id:");
  Serial.println(_id);
  // Write to device.json file 
  fsServices.writeFile(DEVICE_PATH, responseBody.c_str());

}


void Device::launchServer(){}
void Device::connectToWifi(){}

