#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h> // The library for ESP8266WebServer
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <FS.h> // The library for 'SPIFFS'

//WiFi Manager libraries
#include <DNSServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson

#include <PubSubClient.h>

#include "DHTesp.h"
#include <String.h>
#include <SPI.h>

#define DHTpin 14    //D5 of NodeMCU is GPIO14

// maximum received message length
#define MAX_MSG_LEN (128)

DHTesp dht;

long mytime;
float temperatureC;
float humidity;

uint addr = 0;
struct {
  char apikey[32] = "";
  int interval = 0;
  unsigned int enable;
} data;

const char* resource = "/update?api_key=";


// Set the host to the esp8266 file system
const char* host = "esp2866fs";


// Set the ESP8266 Web Server to port 80
ESP8266WebServer espServer(80);


//  Wifi managerr access point name and password
char* APName = "AutoConnectAP";
char* APPassword = "password";
char* SSID;

//flag for saving data
bool shouldSaveConfig = false;

// MQTT Configuration
// MQTT server hostname
// Thing Speak API server
const char* server = "mqtt.thingspeak.com";

// If no MQTT server available you can use an IP address like this
//const IPAddress serverIPAddress(192, 168, 0, 11);

// the topic we want to use
//const char *topic = "TestTopic";
const char *topic = "channels/810424/publish/HK4TKX5WA8KGV27G";
WiFiClient espClient;
PubSubClient client(espClient);

char writeAPIKey[] = "HK4TKX5WA8KGV27G";    // Change to your channel write API key.
long channelID = 810424;
static const char alphanum[] = "0123456789"
                               "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                               "abcdefghijklmnopqrstuvwxyz";  // For random generation of client ID.

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

// Read configration file
void ReadConfig() {
  if (SPIFFS.exists("/config.json")) {
    //file exists, reading and loading
    Serial.println("reading config file");
    File configFile = SPIFFS.open("/config.json", "r");
    if (configFile) {
      Serial.println("opened config file");
      size_t size = configFile.size();
      // Allocate a buffer to store contents of the file.
      std::unique_ptr<char[]> buf(new char[size]);

      configFile.readBytes(buf.get(), size);
      DynamicJsonBuffer jsonBuffer;
      JsonObject& json = jsonBuffer.parseObject(buf.get());
      json.printTo(Serial);
      if (json.success()) {
        Serial.println("\nparsed json");

      } else {
        Serial.println("failed to load json config");
      }
      configFile.close();
    }
  }
}

void WiFiManagerSetup() {
  //read configuration from FS json
  Serial.println("mounting FS...");

  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    ReadConfig();
  } else {
    Serial.println("failed to mount FS");
  }
  //end read


  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect(APName, APPassword)) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }

  //if you get here you have connected to the WiFi
  Serial.println("connected...successfully :)");

  /*
    //save the custom parameters to FS
    if (shouldSaveConfig) {
      Serial.println("saving config");
      DynamicJsonBuffer jsonBuffer;
      JsonObject& json = jsonBuffer.createObject();
      json["http_server"] = http_server;
      json["http_port"] = http_port;
      json["username"] = username;
      json["userpassword"] = userpassword;

      File configFile = SPIFFS.open("/config.json", "w");
      if (!configFile) {
        Serial.println("failed to open config file for writing");
      }

      json.printTo(Serial);
      json.printTo(configFile);
      configFile.close();
      //end save
      ReadConfig();
    }
  */
}

//format bytes size to known bytes units.
String formatBytes(size_t bytes) {
  if (bytes < 1024) {
    return String(bytes) + "B";
  } else if (bytes < (1024 * 1024)) {
    return String(bytes / 1024.0) + "KB";
  } else if (bytes < (1024 * 1024 * 1024)) {
    return String(bytes / 1024.0 / 1024.0) + "MB";
  } else {
    return String(bytes / 1024.0 / 1024.0 / 1024.0) + "GB";
  }
}

// check for the file extension to get the file type.
String getContentType(String filename) {
  if (espServer.hasArg("download")) {
    return "application/octet-stream";
  } else if (filename.endsWith(".htm")) {
    return "text/html";
  } else if (filename.endsWith(".html")) {
    return "text/html";
  } else if (filename.endsWith(".css")) {
    return "text/css";
  } else if (filename.endsWith(".js")) {
    return "application/javascript";
  } else if (filename.endsWith(".png")) {
    return "image/png";
  } else if (filename.endsWith(".gif")) {
    return "image/gif";
  } else if (filename.endsWith(".jpg")) {
    return "image/jpeg";
  } else if (filename.endsWith(".ico")) {
    return "image/x-icon";
  } else if (filename.endsWith(".xml")) {
    return "text/xml";
  } else if (filename.endsWith(".pdf")) {
    return "application/x-pdf";
  } else if (filename.endsWith(".zip")) {
    return "application/x-zip";
  } else if (filename.endsWith(".gz")) {
    return "application/x-gzip";
  }
  return "text/plain";
}

// Read the file
bool handleFileRead(String path) {
  Serial.println("handleFileRead: " + path);
  // If the path is the root add the index.htm to it.
  if (path.endsWith("/")) {
    Serial.println("path ends With / " + path);
    path += "settings.htm";
  }
  // call the getContentType method and set the result to string varible.
  String contentType = getContentType(path);
  // Compress the file
  String pathWithGz = path + ".gz";

  //Check if the file exist on the flash file system zip or unzip.
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
    // Check again if the ziped file exist. I don't understand why and why zip it again!?
    if (SPIFFS.exists(pathWithGz)) {
      path += ".gz";
    }
    // Open the file in read mode
    File file = SPIFFS.open(path, "r");
    espServer.streamFile(file, contentType);
    // close the file.
    file.close();
    return true;
  }
  return false;
}

void setup() {

  Serial.begin(115200);

  dht.setup(DHTpin, DHTesp::DHT11); //for DHT11 Connect DHT sensor to GPIO 17

  // commit 512 bytes of ESP8266 flash (for "EEPROM" emulation)
  // this step actually loads the content (512 bytes) of flash into
  // a 512-byte-array cache in RAM
  EEPROM.begin(512);

  // read bytes (i.e. sizeof(data) from "EEPROM"),
  // in reality, reads from byte-array cache
  // cast bytes into structure called data
  EEPROM.get(addr, data);
  //if(data.apikey != "" && data.interval > 0){
  //  APIKey = String(data.apikey);
  //  timeInterval = int(data.interval);

  Serial.println("Values are: " + String(data.apikey) + "," + String(data.interval));
  //}


  //Start flash file system
  SPIFFS.begin();
  {
    // Open the direction
    Dir dir = SPIFFS.openDir("/");

    // Get the files names and sizes
    while (dir.next()) {
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      Serial.printf("FS File: %s, Size: %s\n", fileName.c_str(), formatBytes(fileSize).c_str());
    }

  }
  WiFiManagerSetup();
  mytime = millis();

  // Start mdns for the file system
  MDNS.begin(host);
  Serial.print("Open http://");
  Serial.print(host);
  Serial.println(".local to see the home page");
  espServer.onNotFound([]() {
    if (!handleFileRead(espServer.uri())) {
      espServer.send(404, "text/plain", "FileNotFound");
    }
  });
  espServer.on("/getSettings", HTTP_GET, bindValues);

  //Configuring the web server
  espServer.on("/settings", HTTP_POST, response);


  // start the server
  espServer.begin();
  Serial.println("espServer server started");

  client.setServer(server, 1883);   // Set the MQTT broker details.
  Serial.println("Set the MQTT broker details");

}

void loop() {


  espServer.handleClient();
  //  Serial.println("timeInterval is: " + String(data.interval));
  //  Serial.println("api key is: " + String(data.apikey));
  //  Serial.println("enable is: " + String(data.enable));
  //  Serial.println("millis - mytime is: " + String(millis() - mytime));
  //delay(500);
  MDNS.update();

  //  delay(500);
  //  Serial.println("mytime Is: " + String(mytime));
  //  Serial.println("millis is: " + String(millis()));

  // make the request if the interval is valid
  if ((millis() - mytime) > (data.interval * 1000) && data.interval >= 30) {

    mytime = millis();
    //    Serial.println("timeInterval condition is: " + String(data.interval));
    //    Serial.println("millis() - mytime is: " + String(millis() - mytime));

    if (data.enable == 1) {
      Serial.println("enable is: true" );
      connectMQTT();
      // makeHTTPRequest();
    }
  }
}

void response() {
  Serial.println("In response arg is: " );
  if (espServer.hasArg("submit")) {
    Serial.print("submit arg:\t");
    Serial.println(espServer.arg("submit"));
  }
  // Check if the api key not null and not more than 20 char.
  if (espServer.hasArg("apiKey") && (espServer.arg("apiKey").length() > 0)) {
    if ((espServer.arg("apiKey").length() > 20)) {
      return espServer.send(500, "text/plain", "BAD ARGS");
    }
    Serial.print("User entered:\t");
    Serial.println(espServer.arg("apiKey"));
    espServer.arg("apiKey").toCharArray(data.apikey, 32);
  }
  else {
    return espServer.send(500, "text/plain", "BAD ARGS");
  }
  // Check for interval
  if (espServer.hasArg("interval") && (espServer.arg("interval").length() > 0)) {
    Serial.print("User entered:\t");
    data.interval =  espServer.arg("interval").toInt();
    Serial.println(data.interval);
  }
  else {
    return espServer.send(500, "text/plain", "BAD ARGS");
  }
  // Check for enable status, no check for length here because if it > 0 it will be true all the time.
  if (espServer.hasArg("checky")) {
    Serial.print("User cecked:\t");
    Serial.println(espServer.arg("checky"));
    if (espServer.arg("checky") == "0") {
      data.enable = 0;
      Serial.print("User cecked false: " + data.enable);
    }
    else if (espServer.arg("checky") == "1") {
      Serial.print("User cecked true: " + data.enable);
      data.enable = 1;
      Serial.print("User cecked:\t");
    }
  }
  else {
    data.enable = 0;
    Serial.print("User cecked false: " + data.enable);
  }
  struct {
    String val = "";
    int interval = 0;
  } ldata;

  // commit 512 bytes of ESP8266 flash (for "EEPROM" emulation)
  // this step actually loads the content (512 bytes) of flash into
  // a 512-byte-array cache in RAM
  EEPROM.begin(512);

  // replace values in byte-array cache with modified data
  // no changes made to flash, all in local byte-array cache
  EEPROM.put(addr, data);

  // actually write the content of byte-array cache to
  // hardware flash.  flash write occurs if and only if one or more byte
  // in byte-array cache has been changed, but if so, ALL 512 bytes are
  // written to flash
  EEPROM.commit();

  Serial.println("In Response Values are: " + String(data.apikey) + "," + String(data.interval) + "," + String(data.enable));
  //delay(500);
  handleFileRead("/success.htm");

}


void bindValues() {
  Serial.println("\nin bindValues");

  String json = "{";
  json += "\"apiKey\": \"" + String(data.apikey) + "\" ,\"interval\":" + data.interval + ",\"enableD\":" + data.enable;
  json += "}";
  // send the json
  espServer.send(200, "text/json", json);

  Serial.println("\nending bindValues");
}

// connect to MQTT server
void connectMQTT() {
  delay(dht.getMinimumSamplingPeriod());
  // Read the sensor data - temperature and humidity
  temperatureC = dht.getTemperature();
  humidity = dht.getHumidity();
  // print the temperature and humidity values on the serial window
  Serial.println("Temperature is: ");
  Serial.print(temperatureC);
  Serial.println(" degrees C");
  Serial.println("Humidity is: ");
  Serial.println(humidity);
  char clientID[9];

  // Wait until we're connected
  while (!client.connected()) {
    // Create a random client ID
    /* String clientId = "ESP8266-";
      clientId += String(random(0xffff), HEX);
      Serial.printf("MQTT connecting as client %s...\n", clientId.c_str());  */
    Serial.println("Attempting MQTT connection...");
    // Generate ClientID
    for (int i = 0; i < 8; i++) {
      clientID[i] = alphanum[random(51)];
    }

    clientID[8] = '\0';

    // Attempt to connect
    /* if (client.connect(clientId.c_str())) */
    //if (client.connect(clientID, mqttUserName, mqttPass))
    if (client.connect(clientID))
    {
      Serial.println("MQTT connected");
      // Once connected, publish an announcement...
      //client.print(String("GET ") + resource + String(data.apikey) + "&field1=" + temperatureC  + "&field2=" + humidity +
      //               " HTTP/1.1\r\n" +
      //               "Host: " + server + "\r\n" +
      //               "Connection: close\r\n\r\n");
      char message[128];
      char temperat[8];
      char humid[8];
      strcpy(message, "field1=");

      snprintf(temperat, sizeof temperat, "%f", temperatureC);
      strcat(message, temperat);
      strcat(message, "&field2=");
      snprintf(humid, sizeof humid, "%f", humidity);
      strcat(message, humid);
      Serial.printf(message);
      client.publish(topic, message);
      mytime = millis();
      //client.publish(topic, "field1=" + temperatureC  + "&field2=" + humidity);
      // ... and resubscribe
      client.subscribe(topic);
    } else {
      Serial.printf("MQTT failed, state %s, retrying...\n", client.state());
      // Wait before retrying
      delay(2500);
    }
  }
}
