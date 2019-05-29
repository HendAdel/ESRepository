/**
   BasicHTTPClient.ino

    Created on: 24.05.2015

*/

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>

#include <WiFiClient.h>

//#include <iostream>   // std::cout
//#include <string>     // std::string, std::stof
#define FIRMWARE_VERSION "1.4"
ESP8266WiFiMulti WiFiMulti;

void readFloat() {
  int i;
  float val;
  char buff[10];

  //Serial.begin(115200);
  val = 1.4;
  for (i = 0; i < 10; i++) {
    snprintf (buff, sizeof(buff), "%f", val);
    Serial.print("val: ");
    Serial.println(val);
    //val += 5.0;
  }

  Serial.println("Contents of Buffer: ");
  for (int a = 0; a < 10; a++) {
    Serial.println(buff[a]);
  }

}

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
  WiFiMulti.addAP("Focus", "Focus@Pro");
  //readFloat();
}

void loop() {
  // wait for WiFi connection
  if ((WiFiMulti.run() == WL_CONNECTED)) {

    WiFiClient client;

    HTTPClient http;

    Serial.print("[HTTP] begin...\n");
    if (http.begin(client, "http://www.the-diy-life.co/Firmware_Version3.txt")) {  // HTTP

      
      Serial.print("[HTTP] GET...\n");
      // start connection and send HTTP header
      int httpCode = http.GET();

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          char Cversion_buff[] = FIRMWARE_VERSION;
          char payload_buff[10];
          String payload = http.getString();
          payload.toCharArray(payload_buff,10);

          
          Serial.println(payload);
          String Cversion = "1.4";
//          char temp[Cversion.length() + 1];
//          char tab2[payload.length() + 1];
//          strcpy (tab2, payload.c_str());
//          strcpy (temp, Cversion.c_str());
//          Serial.printf("Server version: %d\n", payload.c_str());
//          Serial.printf("Curent version: %d\n", Cversion.c_str());
//          Serial.printf("Server version in char: %d\n", tab2);
//          Serial.printf("Curent version in char: %d\n", temp);
         //if (strcmp(Cversion_buff, payload_buff) == 0) {
         // if (payload.equals(Cversion)) {
         if(payload == "1.4"){
            Serial.printf("Version is Updated.");
          }
          else {
            Serial.printf("Version is not equal :\n");
          }
        }
      } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }

      http.end();
    } else {
      Serial.printf("[HTTP} Unable to connect\n");
    }
  }

  delay(10000);
}
