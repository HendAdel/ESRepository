
// Common libraries
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

//WiFi Manager libraries
#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <DNSServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson

//HttpClient libraries
#include <WiFiClient.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>

#include <ESP8266httpUpdate.h>

// New definitions

char http_server[40];
char http_port[6] = "8080";
char username[34] = "admin";
char userpassword[34] = "admin";
char* APName = "AutoConnectAP";
char* APPassword = "password";
char* SSID;
char* password;

//flag for saving data
bool shouldSaveConfig = false;

// The definitions from WiFi Manager
/*
  //define your default values here, if there are different values in config.json, they are overwritten.
  char http_server[40];
  char http_port[6] = "8080";
  char blynk_token[34] = "YOUR_BLYNK_TOKEN";

  //flag for saving data
  bool shouldSaveConfig = false;

  // For first test
  char* APName = "AutoConnectAP";
  char* APPassword = "password";
  char* StaName = "Profit";
  char* StaPassword = "Pro.Profit";
*/

// The definitions from http client
/*
  #ifndef STASSID
  #define STASSID "Focus"
  #define STAPSK  "Focus@Pro"
  #endif

  const char* ssid = STASSID;
  const char* password = STAPSK;
*/
ESP8266WebServer server(90);
ESP8266WiFiMulti WiFiMulti;

WiFiClient client;

HTTPClient http;

#define FIRMWARE_VERSION "1.4.5"

#define USE_SERIAL Serial

// Methods from WiFi Manager
//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

// Methods from Http Update

bool read_file() {

  bool updated; if (http.begin(client, "http://www.the-diy-life.co/Firmware_Version")) {  // HTTP

    Serial.print("[HTTP] GET...\n");
    // start connection and send HTTP header
    int httpCode = http.GET();
    char payload_buff[10];
    char Cversion_buff[10] = FIRMWARE_VERSION;

    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {

        String payload = http.getString();
        Serial.println("server version is: " + payload);
        Serial.printf("control version is: %s\n", Cversion_buff);
        payload.toCharArray(payload_buff, 10);
        if (strcmp(Cversion_buff, payload_buff) == 0) {
          updated = true;
          Serial.println("server version is updated");
        }
        else {
          updated = false;
          Serial.println("server version is not updated");
        }
      }
    }
    else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  } else {
    Serial.printf("[HTTP} Unable to connect\n");
  }
  return updated;
}
//------------------------------------------------------------------------------------------------------------------------

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
        /*
          char username[34] = "admin";
          char userpassword[34] = "admin";*/
        strcpy(http_server, json["http_server"]);
        strcpy(http_port, json["http_port"]);
        strcpy(username, json["username"]);
        strcpy(userpassword, json["userpassword"]);
      } else {
        Serial.println("failed to load json config");
      }
      configFile.close();
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println();

  // WiFi Manager setup code ****************************************** Start****************************
  //read configuration from FS json
  Serial.println("mounting FS...");

  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    ReadConfig();
  } else {
    Serial.println("failed to mount FS");
  }
  //end read



  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  WiFiManagerParameter custom_http_server("server", "http server", http_server, 40);
  WiFiManagerParameter custom_http_port("port", "http port", http_port, 6);
  WiFiManagerParameter custom_username("username", "username", username, 32);
  WiFiManagerParameter custom_userpassword("userpassword", "userpassword", userpassword, 32);

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  //set static ip
  //wifiManager.setSTAStaticIPConfig(IPAddress(10,0,1,99), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

  //add all your parameters here
  wifiManager.addParameter(&custom_http_server);
  wifiManager.addParameter(&custom_http_port);
  wifiManager.addParameter(&custom_username);
  wifiManager.addParameter(&custom_userpassword);

  //reset settings - for testing
  //wifiManager.resetSettings();

  //set minimu quality of signal so it ignores AP's under that quality
  //defaults to 8%
  //wifiManager.setMinimumSignalQuality();

  //sets timeout until configuration portal gets turned off
  //useful to make it all retry or go to sleep
  //in seconds
  //wifiManager.setTimeout(120);

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
  Serial.println("connected...yeey :)");
  //read updated parameters
  strcpy(http_server, custom_http_server.getValue());
  strcpy(http_port, custom_http_port.getValue());
  strcpy(username, custom_username.getValue());
  strcpy(userpassword, custom_userpassword.getValue());

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

    // Http update code ########################################## Start ######################################

    // After connect Write a message on the screen with the IP address
    Serial.println("Uploaded  Success");
    Serial.print("IP Address is: ");
    Serial.println(WiFi.localIP());

    // Http update code ########################################## End ######################################


  }

  //  Serial.println("local ip");
  //  Serial.println(WiFi.localIP());
  // WiFi Manager setup code ****************************************** End ****************************
}

void loop() {

  server.handleClient();

  // The line below is optional. It can be used to blink the LED on the board during flashing
  // The LED will be on during download of one buffer of data from the network. The LED will
  // be off during writing that buffer to flash
  // On a good connection the LED should flash regularly. On a bad connection the LED will be
  // on much longer than it will be off. Other pins than LED_BUILTIN may be used. The second
  // value is used to put the LED on. If the LED is on with HIGH, that value should be passed
  ESPhttpUpdate.setLedPin(2, LOW);
  bool  updated = read_file();
  Serial.println("returend value from the method is: " + updated);
  if (updated == false) {
    Serial.println("updated = false");
    t_httpUpdate_return ret = ESPhttpUpdate.update(client, "server/file.bin");
    // Or:
    //t_httpUpdate_return ret = ESPhttpUpdate.update(client, "server", 80, "file.bin");
    Serial.println("switch case for the returned result.");
    switch (ret) {
      case HTTP_UPDATE_FAILED:
        USE_SERIAL.printf("HTTP_UPDATE_FAILD Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
        break;

      case HTTP_UPDATE_NO_UPDATES:
        USE_SERIAL.println("HTTP_UPDATE_NO_UPDATES");
        break;

      case HTTP_UPDATE_OK:
        USE_SERIAL.println("HTTP_UPDATE_OK");
        break;
    }
  }
  
}
