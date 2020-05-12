#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include <LittleFS.h>
#include <ESP8266mDNS.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>

#define CONFIG_PATH "/config.json"
#define DEVICE_PATH "/device.json"
// access point
const char *ssid = "ACDevice";
const char *wpa2 = "MD123456789";

String device_id;

// broker
String clientId = "ESP8266Client-";
const char *username = "zwccckwo";
const char *pass = "lm8yAHH_5KWj";
const char *mqtt_server = "m16.cloudmqtt.com";

//api engin
#define AUTH_URL "http://api-engine-v1.herokuapp.com/auth/local"
#define DEVICE_URL "http://api-engine-v1.herokuapp.com/api/v1/devices/"

bool apFlag = true; //use this flage to make sure u run in AP mode or station mode  in loop()
WiFiClient espClient;
PubSubClient client(espClient);

ESP8266WebServer server(80);

const size_t capacityForMyData = JSON_OBJECT_SIZE(1) + 10;
DynamicJsonDocument mydataDoc(capacityForMyData);

//protoType
void setupAP(void);
void launchServer();
void FS_begin();
String readFile(const char *path);
void writeFile(const char *path, const char *message);
int testWifi(void);
int check_config();
int check_device_id();
void reconnect();
void callback(char *topic, byte *payload, unsigned int length);
int sgin_in_and_create_device();
// String message="{\"ssid\":\"test\",\"pass\":\"test\"}";

void handleRoot()
{
  if (!server.hasArg("plain"))
  {
    Serial.println("no body");
    server.send(200, "text/html", "Body not received");
    return;
  }
  else
  {
    String message = server.arg("plain");
    server.send(200, "application/json", message);
    Serial.print("message:");
    Serial.println(message);
    //Todo:save data
    Serial.println("Start Writing  * config.json * file");
    writeFile("/config.json", message.c_str());
    Serial.println("Finish Writing * config.json * file");
    Serial.println("***********************************");
    Serial.println("Start reading  * config.json * file");
    readFile("/config.json");
    Serial.println("Finish reading * config.json * file");
    Serial.println("***********************************");
    Serial.println("rest esp");
    ESP.reset();
  }
}

void setup()
{
  delay(200);
  pinMode(LED_BUILTIN, OUTPUT); 
  digitalWrite(LED_BUILTIN, HIGH); // Turn the LED off by making the voltage HIGH
  Serial.begin(115200);
  Serial.println();
  //*init file system
  FS_begin();
  //*check config file for ssid
  if (check_config() == 20)
  {
    //* check if device is set to db
    //check device.json for device id
    if (check_device_id() == 20)
    {
      // connect to broker
      apFlag = false;
      client.setServer(mqtt_server, 11638);
      client.setCallback(callback);
      Serial.println("finish check_device_id()");
      return;
    }
    // signin withe mail and password  ,  Create new device
    if (sgin_in_and_create_device() == 20)
    {
      apFlag = false;
      client.setServer(mqtt_server, 11638);
      client.setCallback(callback);
      Serial.println("finish sgin_in_and_create_device()");
      return;
    }
  }
  else
  {
    apFlag = true;
    Serial.println("error Can't config ");
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
    if (!client.connected())
    {
      reconnect();
    }
    client.loop();
  }
}

void FS_begin(void)
{
  if (!LittleFS.begin())
  {
    Serial.println("LittleFS mount failed");
    return;
  }
}

String readFile(const char *path)
{
  Serial.printf("Reading file: %s\n", path);

  File file = LittleFS.open(path, "r");
  if (!file)
  {
    Serial.println("Failed to open file for reading");
    return "";
  }

  Serial.print("Read from file: ");
  String data;
  while (file.available())
  {
    data += char(file.read());
    // Serial.write(file.read());
  }
  Serial.println(data);
  file.close();
  return data;
}
void writeFile(const char *path, const char *message)
{
  Serial.printf("Writing file: %s\n", path);

  File file = LittleFS.open(path, "w");
  if (!file)
  {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message))
  {
    Serial.println("File written");
  }
  else
  {
    Serial.println("Write failed");
  }
  delay(2000); // Make sure the CREATE and LASTWRITE times are different
  file.close();
}

void setupAP(void)
{
  Serial.print("Configuring access point...");
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
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
  WiFi.softAP(ssid, wpa2);
  Serial.println("softap");
  Serial.println("");
  Serial.println(WiFi.softAPIP());
  launchServer();
  Serial.println("over");
}

void launchServer(void)
{

  Serial.println("");
  Serial.println("launchServer");
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
  // Start the server
  server.on("/", handleRoot);
  server.begin();
  Serial.println("HTTP server started");
  MDNS.addService("http", "tcp", 80);
}

int check_config(void)
{
  // const char* json = "{\"ssid\":\"\",\"wpa2\":\"\",\"email\":\"\",\"password\":\"\"}";

  //check if device have config file
  if (LittleFS.exists(CONFIG_PATH))
  {
    Serial.println("File config.js is exits");
    //check if there is ssid
    String json = readFile(CONFIG_PATH);
    const size_t capacity = JSON_OBJECT_SIZE(4) + 120;
    DynamicJsonDocument doc(capacity);
    DeserializationError error = deserializeJson(doc, json);
    if (error)
    {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
      goto SETUPAP;
    }
    String net_ssid = doc["ssid"]; // network SSID
    String net_wpa2 = doc["wpa2"]; // network password
    if (net_ssid.length() > 1)
    {
      // connect to wifi
      WiFi.begin(net_ssid.c_str(), net_wpa2.c_str());
      if (testWifi() == 20)
      {
        Serial.println("device connected to Network ");
        return 20;
      }
    }
  }
  // if not create AP and lanch server
SETUPAP:
  setupAP();
  return 10;
}

int check_device_id(void)
{
  //  const char* json = "{\"_id\":\"5e922ff9cbb28432f45c4d7d\"}";
  //check if device have config file
  if (LittleFS.exists(DEVICE_PATH))
  {
    Serial.println("File device.js is exits");
    //check if there is ssid
    String json = readFile(DEVICE_PATH);
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
      device_id = _id;
      return 20;
    }
  }
  return 10;
}

int testWifi(void)
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
    Serial.print(WiFi.status());
    c++;
  }
  Serial.println("Connect timed out, opening AP");
  return (10);
}

void reconnect(void)
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("MQTT connection...");
    // Create a random client ID
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), username, pass))
    {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("api-engine", "device connected");
      // ... and resubscribe    5e9124c9d80c373888e048ba
      String topic = "devices/" + device_id;
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

void callback(char *topic, byte *payload, unsigned int length)
{
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
  String value = mydataDoc["value"]; // "on"
  Serial.print("value: ");
  Serial.println(value);
  // Switch on the LED if an 1 was received as first character
  if (value == "on")
  {
    digitalWrite(LED_BUILTIN, LOW); // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  }
  else
  {
    digitalWrite(LED_BUILTIN, HIGH); // Turn the LED off by making the voltage HIGH
  }
}

int sgin_in_and_create_device()
{
  const size_t cap = JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(5) + 600;
  DynamicJsonDocument res(cap);

  //read ueer eamil pass from comfig file
  String json = readFile(CONFIG_PATH);
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

  const size_t capacity2 = JSON_OBJECT_SIZE(2)+50;
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
  http.begin(client, AUTH_URL); //HTTP
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
      Serial.println("received payload:\n<<");
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
  // configure traged server and url
  http.begin(client, DEVICE_URL); //HTTP
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
      Serial.println("received payload:\n<<");
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
  device_id = _id;
  Serial.print("device_id:");
  Serial.println(device_id);
  //*write to device.json file
  writeFile(DEVICE_PATH, message.c_str());
  return 20;
}


//* user json */

// const size_t cap = JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(5) + 450;
// DynamicJsonDocument res(cap);

// const char* json = "{\"user\":{\"_id\":\"5e9411c1db92770004bd4a0e\",\"name\":\"mohamed\",\"email\":\"test@gmail.com\",\"picture\":\"https://external-content.duckduckgo.com/iu/?u=https%3A%2F%2Fimage.tmdb.org%2Ft%2Fp%2Foriginal%2FoqvusJfmH4zN2LgdCjmB2TxetOd.jpg&f=1&nofb=1\",\"role\":\"user\"},\"token\":\"eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJfaWQiOiI1ZTk0MTFjMWRiOTI3NzAwMDRiZDRhMGUiLCJpYXQiOjE1ODkxMjgyOTYsImV4cCI6MTU4OTIxNDY5Nn0.-WeRiOgJc5Nk3EDbl6iI83RUGt8xgjm1YHDgIYW95xY\"}";

// deserializeJson(res, json);

// JsonObject user = doc["user"];
// const char* user__id = user["_id"]; // "5e9411c1db92770004bd4a0e"
// const char* user_name = user["name"]; // "mohamed"
// const char* user_email = user["email"]; // "test@gmail.com"
// const char* user_picture = user["picture"]; // ""
// const char* user_role = user["role"]; // "user"

// const char* token = res["token"]; // "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJfaWQiOiI1ZTk0MTFjMWRiOTI3NzAwMDRiZDR0

//** device json*/
// const size_t deviceCapacity = JSON_OBJECT_SIZE(12) + 260;
// DynamicJsonDocument doc(deviceCapacity);

// const char* json = "{\"_id\":\"5e922ff9cbb28432f45c4d7d\",\"createdAt\":\"2020-04-11T21:00:41.065Z\",\"updatedAt\":\"2020-05-05T14:43:06.739Z\",\"macAddress\":\"AA:AA:AA:AA\",\"tag\":\"AC\",\"version\":1,\"intensity\":100,\"email\":\"test@test.com\",\"value\":\"on\",\"__v\":0,\"name\":\"lamp2\",\"createdBy\":\"5e9411c1db92770004bd4a0e\"}";

// deserializeJson(deviceData, json);

// const char* _id = doc["_id"]; // "5e922ff9cbb28432f45c4d7d"
// const char* createdAt = doc["createdAt"]; // "2020-04-11T21:00:41.065Z"
// const char* updatedAt = doc["updatedAt"]; // "2020-05-05T14:43:06.739Z"
// const char* macAddress = doc["macAddress"]; // "AA:AA:AA:AA"
// const char* tag = doc["tag"]; // "AC"
// int version = doc["version"]; // 1
// int intensity = doc["intensity"]; // 100
// const char* email = doc["email"]; // "test@test.com"
// const char* value = doc["value"]; // "on"
// int _v = doc["__v"]; // 0
// const char* name = doc["name"]; // "lamp2"
// const char* createdBy = doc["createdBy"]; // "5e9411c1db92770004bd4a0e"