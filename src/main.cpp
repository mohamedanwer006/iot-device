#include "constant.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "mqttServices.h"
#include "device.h"

String deviceId = ""; 
// This flage  make sure u run in AP mode or station mode  in loop()
bool apFlag = true; 

MqttServices mqttServices = MqttServices(MQTT_SERVER,USERNAME,PASSWORD );

ESP8266WebServer server(80);

const size_t capacityForMyData = JSON_OBJECT_SIZE(1) + 10;
DynamicJsonDocument mydataDoc(capacityForMyData);

// function protoType
void handleRoot();
void setupAP(void);
void launchServer();
int checkConnection(void);
int check_config();
int checkDeviceId();
int signInAndCreateDevice();

FsServices fsServices;
Device device;
void setup()
{
  device.init();
  //* check config file for ssid
  mqttServices.connect();
  if (check_config() == 20)
  {
    //* check if device is set to db
    // check device.json for device id
    if (checkDeviceId() == 20)
    {
      // if there is id in file
      //* connect to broker
      apFlag = false;
      // client.setServer("", 11638);
      // client.setCallback(callback);
      Serial.println("Finish check device id ");
      return;
    }
    //* signing withe mail and password  ,  Create new device
    if (signInAndCreateDevice() == 20)
    {
      apFlag = false;
      // client.setServer("mqtt_server", 11638);
      // client.setCallback(callback);
      Serial.println("Sign in and create device()");
      return;
    }
  }
  else
  {
    apFlag = true;
    Serial.println("Error Can't config ");
  }
  apFlag = true;
  return;
}
void loop()
{
  if (apFlag)
  {
    MDNS.update();
    server.handleClient();
  }
  else
  {
    
    mqttServices.loop(deviceId);
  }
}

void setupAP(void)
{
  Serial.print("Configuring access point...");
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  // scan for the WiFi networks 
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
      // Print SSID and RSSI for each network found
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
  delay(100);
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  Serial.println("softap");
  Serial.println("");
  Serial.println(WiFi.softAPIP());
  launchServer();
  Serial.println("over");
}

void launchServer(void)
{

  Serial.println("");
  Serial.println("Launch server");
  Serial.println(WiFi.softAPIP());

  if (!MDNS.begin("esp8266", WiFi.softAPIP()))
  {
    Serial.println("Error setting up MDNS responder!");
    while (1)
    {
      delay(1000);
    }
  }

  Serial.println("mDNS responder started");
  Serial.println("->->->-> http://esp8266.local/");
  //* Start the server
  server.on("/", handleRoot);
  server.begin();
  Serial.println("HTTP server started");
  MDNS.addService("http", "tcp", 80);
}

int check_config(void)
{
  // const char* json = "{\"ssid\":\"\",\"wpa2\":\"\",\"email\":\"\",\"password\":\"\"}";
  // check config file
  if (fsServices.isFileExist(CONFIG_PATH))
  {
    Serial.println("File config.js is exits");
    // check ssid
    String json = fsServices.readFile(CONFIG_PATH);
    const size_t capacity = JSON_OBJECT_SIZE(4) + 120;
    DynamicJsonDocument doc(capacity);
    DeserializationError error = deserializeJson(doc, json);
    if (error)
    {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
      // create AP and launch server
      setupAP();
      return 10;
    }
    String net_ssid = doc["ssid"]; // network SSID
    String net_wpa2 = doc["wpa2"]; // network password
    if (net_ssid.length() > 1)
    {
      // connect to wifi
      WiFi.begin(net_ssid.c_str(), net_wpa2.c_str());
       Serial.print("Waiting while connecting to  : ");
       Serial.println(net_ssid);
      if (checkConnection() == 20)
      {
        Serial.println("");
        Serial.println("Device connected to Network ");
        return 20;
      }
      else
      {
        Serial.println("Device can't connect to Network ");
        setupAP();
        return 10;
      }
    }
    else // create AP and launch server
    {
      setupAP();
      return 10;
    }
  }
  else
  {
    Serial.println("config.json is not exits");
    setupAP();
    return 10;
  }
}

int checkDeviceId(void)
{
  //  const char* json = "{\"_id\":\"5e922ff9cbb28432f45c4d7d\"}";
  // device configuration file
  if (fsServices.isFileExist(DEVICE_PATH))
  {
    Serial.println("File : device.json = OK ");
    String json = fsServices.readFile(DEVICE_PATH);
    const size_t capacity = JSON_OBJECT_SIZE(6) + 200;
    DynamicJsonDocument doc(capacity);
    DeserializationError error = deserializeJson(doc, json);
    if (error)
    {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
      return 10;
    }
    String _id = doc["_id"]; // "5e922ff9cbb28432f45c4d7d"
    if (_id.length() > 1)
    {
      deviceId = _id;
      return 20;
    }
  }
  return 10;
}

int checkConnection(void)
{
  int c = 0;
  Serial.println("Waiting for Wifi to connect");
  while (c < 20)
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      return (20);
    }
    delay(500);
    // Serial.print(WiFi.status());
    Serial.print("#");
    c++;
  }
  Serial.println("Connect timed out, opening AP");
  return (10);
}

int signInAndCreateDevice()
{
  const size_t cap = JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(5) + 600;
  DynamicJsonDocument res(cap);

  // Read user email  and passwd from config file
  String json = fsServices.readFile(CONFIG_PATH);
  const size_t capacity1 = JSON_OBJECT_SIZE(4) + 400;
  DynamicJsonDocument doc(capacity1);
  DeserializationError error = deserializeJson(doc, json);
  if (error)
  {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
  }

  String email = doc["email"];       // network SSID
  String password = doc["password"]; // network password

  const size_t capacity2 = JSON_OBJECT_SIZE(2) + 50;
  DynamicJsonDocument docs(capacity2);

  docs["email"] = email;
  docs["password"] = password;

  String body;
  serializeJson(docs, body);
  Serial.println(body);
  WiFiClient client;
  HTTPClient http;

  Serial.print("[HTTP] begin...\n");
  // configure traged server and url
  http.begin(client, AUTH_URL); // HTTP
  http.addHeader("Content-Type", "application/json");
  // http.addHeader("Authorization", "Bearer ");
  Serial.print("[HTTP] POST...\n");
  // start connection and send HTTP header and body
  int httpCode = http.POST(body);
  // httpCode will be negative on error
  if (httpCode > 0)
  {
    // HTTP header has been send and Server response header has been handled
    Serial.printf("[HTTP] POST... code: %d\n", httpCode);
    // file found at server
    if (httpCode == HTTP_CODE_OK)
    {
      const String &payload = http.getString();
      Serial.println("Received payload:\n<<");
      Serial.println(payload);
      deserializeJson(res, payload);
      Serial.println(">>");
    }
  }
  else
  {
    Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
  http.end();

  String token = res["token"];
  String authtoken = "Bearer " + token;
  Serial.print("Authorization:");
  Serial.println(authtoken);
  Serial.print("[HTTP] begin...\n");
  const size_t deviceCapacity = JSON_OBJECT_SIZE(12) + 400;
  DynamicJsonDocument deviceData(deviceCapacity);
  String deviceBody = "{\"name\":\"new device\"}";
  String message;
  // Connect to API and add new device
  http.begin(client, DEVICE_URL); // HTTP
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", authtoken);
  Serial.print("[HTTP] POST...\n");
  // start connection and send HTTP header and body
  httpCode = http.POST(deviceBody);
  // httpCode will be negative on error
  if (httpCode > 0)
  {
    // HTTP header has been send and Server response header has been handled
    Serial.printf("[HTTP] POST... code: %d\n", httpCode);
    // file found at server
    if (httpCode == HTTP_CODE_OK)
    {
      const String &payload = http.getString();
      Serial.println("Received payload:\n<<");
      Serial.println(payload);
      deserializeJson(deviceData, payload);
      message = payload;
      Serial.println(">>");
    }
  }
  else
  {
    Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
  http.end();
  String _id = deviceData["_id"];
  deviceId = _id;
  Serial.print("DeviceID:");
  Serial.println(deviceId);
  // save device data
  fsServices.writeFile(DEVICE_PATH, message.c_str());
  return 20;
}

void handleRoot()
{
  if (!server.hasArg("plain"))
  {
    Serial.println("no body");
    server.send(200, "application/json", "{\"ok\":\"0\"}");
    return;
  }
  else
  {
    String message = server.arg("plain");
    server.send(200, "application/json", "{\"ok\":\"1\"}");
    Serial.print("Message:");
    Serial.println(message);
    Serial.println("Start Writing  * config.json * file");
    fsServices.writeFile("/config.json", message.c_str());
    Serial.println("Finish Writing * config.json * file");
    Serial.println("***********************************");
    Serial.println("Start reading  * config.json * file");
    fsServices.readFile("/config.json");
    Serial.println("Finish reading * config.json * file");
    Serial.println("***********************************");
    Serial.println("Reset Device");
    ESP.reset();
  }
}
