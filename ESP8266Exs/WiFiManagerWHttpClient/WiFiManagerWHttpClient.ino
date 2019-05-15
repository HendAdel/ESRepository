
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


  // The defntions from WiFi Manager

  //define your default values here, if there are different values in config.json, they are overwritten.
  char http_server[40];
  char http_port[6] = "8080";
  char blynk_token[34] = "YOUR_BLYNK_TOKEN";

  //flag for saving data
  bool shouldSaveConfig = false;
  char* APName = "AutoConnectAP";
  char* APPassword = "password";
  char* StaName = "Profit";
  char* StaPassword = "Pro.Profit";

   // The defntions from http client
/* 
#ifndef STASSID
#define STASSID "Focus"
#define STAPSK  "Focus@Pro"
#endif

const char* ssid = STASSID;
const char* password = STAPSK;
*/
ESP8266WebServer server(http_port[6]);
ESP8266WiFiMulti WiFiMulti;

// Methods from WiFi Manager
//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

// Methods from Http Client
//Check if header is present and correct
bool is_authenticated() {
  Serial.println("Enter is_authenticated");
  if (server.hasHeader("Cookie")) {
    Serial.print("Found cookie: ");
    String cookie = server.header("Cookie");
    Serial.println(cookie);
    if (cookie.indexOf("ESPSESSIONID=1") != -1) {
      Serial.println("Authentication Successful");
      return true;
    }
  }
  Serial.println("Authentication Failed");
  return false;
}

//login page, also called for disconnect
void handleLogin() {
  String msg;
  if (server.hasHeader("Cookie")) {
    Serial.print("Found cookie: ");
    String cookie = server.header("Cookie");
    Serial.println(cookie);
  }
  if (server.hasArg("DISCONNECT")) {
    Serial.println("Disconnection");
    server.sendHeader("Location", "/login");
    server.sendHeader("Cache-Control", "no-cache");
    server.sendHeader("Set-Cookie", "ESPSESSIONID=0");
    server.send(301);
    return;
  }
  if (server.hasArg("USERNAME") && server.hasArg("PASSWORD")) {
    if (server.arg("USERNAME") == "admin" &&  server.arg("PASSWORD") == "admin") {
      server.sendHeader("Location", "/");
      server.sendHeader("Cache-Control", "no-cache");
      server.sendHeader("Set-Cookie", "ESPSESSIONID=1");
      server.send(301);
      Serial.println("Log in Successful");
      return;
    }
    msg = "Wrong username/password! try again.";
    Serial.println("Log in Failed");
  }
  String content = "<html><body><form action='/login' method='POST'>To log in, please use : admin/admin<br>";
  content += "User:<input type='text' name='USERNAME' placeholder='user name'><br>";
  content += "Password:<input type='password' name='PASSWORD' placeholder='password'><br>";
  content += "<input type='submit' name='SUBMIT' value='Submit'></form>" + msg + "<br>";
  content += "You also can go <a href='/inline'>here</a></body></html>";
  server.send(200, "text/html", content);
}

//root page can be accessed only if authentication is ok
void handleRoot() {
  Serial.println("Enter handleRoot");
  String header;
  if (!is_authenticated()) {
    server.sendHeader("Location", "/login");
    server.sendHeader("Cache-Control", "no-cache");
    server.send(301);
    return;
  }
  String content = "<html><body><H2>hello, you successfully connected to esp8266!</H2><br>";
  if (server.hasHeader("User-Agent")) {
    content += "the user agent used is : " + server.header("User-Agent") + "<br><br>";
  }
  content += "You can access this page until you <a href=\"/login?DISCONNECT=YES\">disconnect</a></body></html>";
  
  server.sendHeader("Set-Cookie", "ESPSESSIONID=0");
  server.send(200, "text/html", content);
}

//no need authentication
void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void setup() {
   Serial.begin(115200);
   Serial.println();

   // WiFi Manager setup code ****************************************** Start****************************
   //read configuration from FS json
   Serial.println("mounting FS...");

   if (SPIFFS.begin()) {
    Serial.println("mounted file system");
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

          strcpy(http_server, json["mqtt_server"]);
          strcpy(http_port, json["mqtt_port"]);
          strcpy(blynk_token, json["blynk_token"]);

        } else {
          Serial.println("failed to load json config");
        }
        configFile.close();
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  //end read



  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  WiFiManagerParameter custom_http_server("server", "http server", http_server, 40);
  WiFiManagerParameter custom_http_port("port", "http port", http_port, 6);
  WiFiManagerParameter custom_blynk_token("blynk", "blynk token", blynk_token, 32);

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
  wifiManager.addParameter(&custom_blynk_token);

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
  if (!wifiManager.autoConnect("AutoConnectAP", "password")) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }

  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");

  // Http client code ########################################## Start ######################################
  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(StaName, StaPassword);
  
  // Http client code ########################################## End ######################################
  
  //read updated parameters
  strcpy(http_server, custom_http_server.getValue());
  strcpy(http_port, custom_http_port.getValue());
  strcpy(blynk_token, custom_blynk_token.getValue());

  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["mqtt_server"] = http_server;
    json["mqtt_port"] = http_port;
    json["blynk_token"] = blynk_token;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }

    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
    //end save
  }

  Serial.println("local ip");
  Serial.println(WiFi.localIP());
  // WiFi Manager setup code ****************************************** End ****************************
}

void loop() {
  // Http Client code
  // wait for WiFi connection
  if ((WiFiMulti.run() == WL_CONNECTED)) {

    WiFiClient client;

  // std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);

    //client->setFingerprint(fingerprint);

   // HTTPClient https;

    Serial.println("");
    Serial.print("Connected! IP address: ");
    Serial.println(WiFi.localIP());
    
    HTTPClient http;
    Serial.print("[HTTP] begin...\n");
    // configure traged server and url
    // http.begin(client, "10.2.101.103");
    http.begin(client, "http://192.168.43.58/iisstart.htm");
    http.setAuthorization("guest", "guest");
    
    //http.begin(client, "http://guest:guest@jigsaw.w3.org/HTTP/Basic/");

    
      // or
      //https.begin(*client, "https://jigsaw.w3.org/HTTP/connection.html");
      
      //or
      //https.begin(*client, "https://accounts.google.com");
     // https.setAuthorization("hend.adel", "D15He4I20Y19nd");
 /*
      http.begin(client, "http://jigsaw.w3.org/HTTP/Basic/");
      http.setAuthorization("guest", "guest");
   

      // or
      http.begin(client, "http://jigsaw.w3.org/HTTP/Basic/");
      http.setAuthorization("Z3Vlc3Q6Z3Vlc3Q=");
      http.setAuthentication   
    */  


    Serial.print("[HTTP] GET...\n");
    // start connection and send HTTP header
    int httpCode = http.GET();

    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println(payload);
      }
    } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  }

  delay(10000);

}
