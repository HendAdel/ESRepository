
// Connect the ESP8266 unit to an existing Wifi access point

#include <ESP8266WiFi.h>

// Set the wifi network settings - name and password -
const char* ssid = "Mi Phone";
const char* password = "0100@Baba579";

void setup() {
  // wait 1 second
  delay(1000);
  //Start serial 
  Serial.begin(115200);

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
  Serial.println("Success!");
  Serial.print("IP Address is: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // put your main code here, to run repeatedly:

}
