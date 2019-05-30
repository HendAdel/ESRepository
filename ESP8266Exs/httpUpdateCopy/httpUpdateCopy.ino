/**
   httpUpdate.ino

    Created on: 27.11.2015

*/

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>

#include <stdio.h>
#include <stdlib.h>

#define FIRMWARE_VERSION "1.4.3"


#define USE_SERIAL Serial

#ifndef APSSID
#define APSSID "Focus"
#define APPSK  "Focus@Pro"
#endif
// Set the wifi network settings - name and password -
const char* ssid = "Focus";
const char* password = "Focus@Pro";
//ESP8266WiFiMulti WiFiMulti;
WiFiClient client;

HTTPClient http;

bool read_file() {

  bool updated; if (http.begin(client, "http://www.the-diy-life.co/Firmware_Version.txt")) {  // HTTP

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

void setup() {

  USE_SERIAL.begin(115200);
  // USE_SERIAL.setDebugOutput(true);

  USE_SERIAL.println();
  USE_SERIAL.println();
  USE_SERIAL.println();

  //  for (uint8_t t = 4; t > 0; t--) {
  //    USE_SERIAL.printf("[SETUP] WAIT %d...\n", t);
  //    USE_SERIAL.flush();
  //    delay(1000);
  //  }
  // start wifi
  WiFi.begin(ssid, password);

  //Write a message on the screen
  Serial.println();
  Serial.print("Connecting");

  // loap to show connecting progress on the screen
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // After connect Write a message on the screen with the IP address
  Serial.println("Uploaded  Success");
  Serial.print("IP Address is: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // wait for WiFi connection
  // if ((WiFiMulti.run() == WL_CONNECTED)) {

    //WiFiClient client;

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
//  }
//  else {
//    Serial.println("No connection");
//    delay(1000);
//
//  }

}
