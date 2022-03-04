#include "wifiServices.h"
#include <ESP8266WiFi.h>

void  WifiServices::connect(const char* ssid,const char *password){
  WiFi.begin(ssid, password);
}

void  WifiServices::disconnect(){
  WiFi.disconnect();
  
}


void  WifiServices::creatAP(const char* ssid,const char *password){
  Serial.print("Configuring access point...");
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
//   delay(100);
  WiFi.softAP(ssid, password);
  Serial.println("softap");
  Serial.println("");
  Serial.println(WiFi.softAPIP());
}


void WifiServices(){
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0)
    Serial.println("no networks found");
  else
  {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i)
    {
      //* Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
      delay(10);
    }
  }
  Serial.println("");
}