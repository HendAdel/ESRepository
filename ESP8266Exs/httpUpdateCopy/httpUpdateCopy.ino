/**
   httpUpdate.ino

    Created on: 27.11.2015

*/

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#define FIRMWARE_VERSION "1.4.3"


#define USE_SERIAL Serial

#ifndef APSSID
#define APSSID "Mi Phone"
#define APPSK  "0100@Baba579"
#endif
// Set the wifi network settings - name and password -
const char* ssid = "Mi Phone";
const char* password = "0100@Baba579";
//ESP8266WiFiMulti WiFiMulti;

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
  while (WiFi.status() != WL_CONNECTED){
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
  //if ((WiFiMulti.run() == WL_CONNECTED)) {

    WiFiClient client;

    // The line below is optional. It can be used to blink the LED on the board during flashing
    // The LED will be on during download of one buffer of data from the network. The LED will
    // be off during writing that buffer to flash
    // On a good connection the LED should flash regularly. On a bad connection the LED will be
    // on much longer than it will be off. Other pins than LED_BUILTIN may be used. The second
    // value is used to put the LED on. If the LED is on with HIGH, that value should be passed
    ESPhttpUpdate.setLedPin(2, LOW);

    t_httpUpdate_return ret = ESPhttpUpdate.update(client, "http://www.the-diy-life.co/download.bin");
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
//  }else {
//      Serial.println("No connection");
//      delay(1000);
//  
//  }
}
