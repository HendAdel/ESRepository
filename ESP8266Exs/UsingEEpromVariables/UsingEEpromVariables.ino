#include <EEPROM.h>

String APIKey;
int timeInterval;

void setup() {
  
  Serial.begin(9600);

  uint addr = 0;

  struct {
    String val = "kiplGuiSvcxzli";
    int interval = 3;
  } data;

  // commit 512 bytes of ESP8266 flash (for "EEPROM" emulation)
  // this step actually loads the content (512 bytes) of flash into
  // a 512-byte-array cache in RAM
  EEPROM.begin(512);

  // replace values in byte-array cache with modified data
  // no changes made to flash, all in local byte-array cache
  EEPROM.put(addr, data);

  // actually write the content of byte-array cache to
  // hardware flash.  flash write occurs if and only if one or more byte
  // in byte-array cache has been changed, but if so, ALL 512 bytes are
  // written to flash
  EEPROM.commit();

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
