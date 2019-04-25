
// Configure the ESP8266 unit as a Wifi access point

#include <ESP8266WiFi.h>
#include <WiFiClient.h>

// Set the wifi soft access point settings - name and password -
const char* ssid = "ESPWiFi";
const char* password = "12345678";

void setup() {
  // wait for a second
  delay(1000);
  // Start the serial
  Serial.begin(115200);
  //Print the progress to the screen
  Serial.println();
  Serial.println();
  Serial.println("Configuring WiFi access point...");

  boolean result = WiFi.softAP(ssid, password);
  Serial.println(result);
  if(result == true){
    IPAddress myIP = WiFi.softAPIP();

    Serial.println("done!");
    Serial.println("");
    Serial.println("WiFi network name: ");
    Serial.println(ssid);
    Serial.println("WiFi network password: ");
    Serial.println(password);
    Serial.println("Host IP Address: ");
    Serial.println(myIP);
    Serial.println("");  
    }
    else{
      Serial.println("error! Somthing went wrong ...");
    }
  /*  
    IPAddress myIP = WiFi.softAPIP();

    Serial.println("done!");
    Serial.println("");
    Serial.println("WiFi network name: ");
    Serial.println(ssid);
    Serial.println("WiFi network password: ");
    Serial.println(password);
    Serial.println("Host IP Address: ");
    Serial.println(myIP);
    Serial.println(""); */
}

void loop() {
  // Print the connected devices numbers on the screen.
  Serial.printf("Number of connected devices (stations) = %d\n", WiFi.softAPgetStationNum());
  //wait 10 second
  delay(10000);
}
