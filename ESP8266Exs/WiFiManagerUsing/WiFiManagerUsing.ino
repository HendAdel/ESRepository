
#include <ESP8266WiFi.h>          //ESP8266 Core WiFi Library (you most likely already have this in your sketch)
#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic


#ifndef STASSID
//#define STASSID "Focus"
//#define STAPSK  "Focus@Pro"
#define STASSID "MiNetWork"
#define STAPSK  "0100@"
#endif

const char* ssid = STASSID;
const char* ssidPassword = STAPSK;

void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());

  Serial.println(myWiFiManager->getConfigPortalSSID());
}

void setup() {
  
  Serial.begin(115200);
  // Create object from wifi manager
  WiFiManager wifiManager;

  wifiManager.setAPCallback(configModeCallback);
  
  wifiManager.autoConnect();


 // WiFi.mode(WIFI_STA);
 // WiFi.begin(ssid, ssidPassword);
/*
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
*/
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  //WiFiManager wifiManager;
  //wifiManager.startConfigPortal("OnDemandAP");
  //Serial.println("connected... :)" + WiFi.softAPIP());
  /*
     if (!wifiManager.startConfigPortal("OnDemandAP")) {
      Serial.println("failed to connect and hit timeout");
      }*/
}

void loop() {
   // is configuration portal requested?
  //if ( digitalRead(TRIGGER_PIN) == LOW ) {
    //WiFiManager wifiManager;
   // wifiManager.startConfigPortal("OnDemandAP");
   // Serial.println("connected... :)" + WiFi.softAPIP());
 // }

}
