#include "mqttServices.h"

static String clientId = "ESP8266Client-";



void MqttServices:: reconnect(String deviceId){

// Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("MQTT connection...");
    // Create a random client ID
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), _username.c_str(), _password.c_str()))
    {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("api-engine", "device connected");
      // ... and resubscribe    5e9124c9d80c373888e048ba
      String topic = "devices/" + deviceId;
      client.subscribe(topic.c_str());
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


bool MqttServices::connected(){
    return client.connected();
}

void MqttServices::loop(String deviceId){
    if (!client.connected())
    {
      this->reconnect(deviceId.c_str());
    }
    client.loop();
}

void MqttServices::connect(){
    Serial.println("Start MQTT connection...");
    client.setServer(_broker.c_str(), 11638);
    client.setCallback(callback);
  
}