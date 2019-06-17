#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h> // The library for ESP8266WebServer

String APIKey;
int timeInterval;
int enable;
long mytime;

  uint addr = 0;
  struct {
    String val = APIKey;//"trewq";
    int interval = timeInterval; //30;
  } data;

// Set the ESP8266 Web Server to port 80
ESP8266WebServer espServer(80);


void setup() {

  Serial.begin(115200);

  // commit 512 bytes of ESP8266 flash (for "EEPROM" emulation)
  // this step actually loads the content (512 bytes) of flash into
  // a 512-byte-array cache in RAM
  EEPROM.begin(512);

  // replace values in byte-array cache with modified data
  // no changes made to flash, all in local byte-array cache

  //EEPROM.put(addr, data);

  // actually write the content of byte-array cache to
  // hardware flash.  flash write occurs if and only if one or more byte
  // in byte-array cache has been changed, but if so, ALL 512 bytes are
  // written to flash

  //EEPROM.commit();

  // read bytes (i.e. sizeof(data) from "EEPROM"),
  // in reality, reads from byte-array cache
  // cast bytes into structure called data
  EEPROM.get(addr, data);
  APIKey = String(data.val);
  timeInterval = int(data.interval);
  Serial.println("Values are: " + String(APIKey) + "," + String(timeInterval));

}

void loop() {
  // put your main code here, to run repeatedly:

}
