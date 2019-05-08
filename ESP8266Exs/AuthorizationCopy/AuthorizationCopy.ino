/**
   Authorization.ino

    Created on: 09.12.2015

*/

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>

#include <WiFiClient.h>
#include <WiFiClientSecureBearSSL.h>

// Fingerprint for demo URL, expires on June 2, 2019, needs to be updated well before this date    
const uint8_t fingerprint[20] = {0x14, 0xEC, 0x6C, 0x23, 0x68, 0x9B, 0x7D, 0xAE, 0x55, 0x70, 0xCB, 0x13, 0x8E, 0x01, 0xD4, 0xEF, 0xB2, 0x12, 0x41, 0x2D};


ESP8266WiFiMulti WiFiMulti;


const char* ssid = "Focus";
const char* password = "Focus@Pro";

//const char* ssid = "Mi Phone";
//const char* password = "0100@Baba579";

void setup() {

  Serial.begin(115200);
  // Serial.setDebugOutput(true);

  Serial.println();
  Serial.println();
  Serial.println();

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(ssid, password);

}

void loop() {
  // wait for WiFi connection
  if ((WiFiMulti.run() == WL_CONNECTED)) {

    //WiFiClient client;

   std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);

    client->setFingerprint(fingerprint);

    HTTPClient https;

   // HTTPClient http;
    Serial.print("[HTTP] begin...\n");
    // configure traged server and url


    //http.begin(client, "http://guest:guest@jigsaw.w3.org/HTTP/Basic/");

    
      // or
      //https.begin(*client, "https://jigsaw.w3.org/HTTP/connection.html");
      https.begin(*client, "https://accounts.google.com");
      https.setAuthorization("hend.adel", "D15He4I20Y19nd");

  /*    http.begin(client, "http://jigsaw.w3.org/HTTP/Basic/");
      http.setAuthorization("guest", "guest");
      
 
      // or
      http.begin(client, "http://jigsaw.w3.org/HTTP/Basic/");
      http.setAuthorization("Z3Vlc3Q6Z3Vlc3Q=");
   */


    Serial.print("[HTTP] GET...\n");
    // start connection and send HTTP header
    int httpCode = https.GET();

    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK) {
        String payload = https.getString();
        Serial.println(payload);
      }
    } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
    }

    https.end();
  }

  delay(10000);
}
