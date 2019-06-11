#include <EEPROM.h>
#include <ESP8266WiFi.h>

String APIKey;
int timeInterval;
long mytime;

const int sensorPin = A0;
const int digitalInput = D1;
int reading;
float voltage;
float temperatureC;
int digitalReader;

// Replace with your SSID and Password
const char* ssid     = "Focus";
const char* password = "Focus@Pro";

// Replace with your unique Thing Speak WRITE API KEY
//const char* apiKey = "K7U14AOS7MM2OUUN";
//First channel api key
//const char* apiKey = "XN1BFT74XNJ18NR1";

const char* resource = "/update?api_key=";

// Thing Speak API server
const char* server = "api.thingspeak.com";

void setup() {

  Serial.begin(9600);

  uint addr = 0;

  struct {
    String val = "K7U14AOS7MM2OUUN";
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
  
  pinMode(digitalInput, INPUT);

  initWifi();
}

void loop() {
  // make the request if the interval is valid
  if ((millis() - mytime) > (timeInterval * 1000)) {
    mytime = millis();
    Serial.println("timeInterval is: " + String(timeInterval));  
    Serial.println("mytime is: " + String(millis() - mytime));  
    digitalReader = digitalRead(digitalInput);
    makeHTTPRequest();
  }
}


// Establish a Wi-Fi connection with your router
void initWifi() {
  Serial.println("Connecting to: ");
  Serial.print(ssid);
  WiFi.begin(ssid, password);

  int timeout = 10 * 4; // 10 seconds
  while (WiFi.status() != WL_CONNECTED  && (timeout-- > 0)) {
    delay(250);
    Serial.print(".");
  }
  Serial.println("");

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Failed to connect, going back to sleep");
  }

  Serial.print("WiFi connected in: ");
  Serial.print(millis());
  Serial.print(", IP address: ");
  Serial.println(WiFi.localIP());
}

// Make an HTTP request to Thing Speak
void makeHTTPRequest() {
  // Read the sensor data
  reading = analogRead(sensorPin);

  // calc the volt
  voltage = reading * (3300 / 1024);
  // print the volts value on the serial window
  Serial.print(voltage);
  Serial.println(" volts");

  // calc the Temperature from the volts
  temperatureC = (voltage - 500) / 100;

  // print the volts value on the serial window
  Serial.println("Temperature is: ");
  Serial.print(temperatureC);
  Serial.println(" degrees C");
  Serial.println("Digital input is: ");
  Serial.print(digitalReader);

  // wait 10 second
  delay(10000);

  Serial.print("Connecting to ");
  Serial.print(server);

  WiFiClient client;
  int retries = 5;
  while (!!!client.connect(server, 80) && (retries-- > 0)) {
    Serial.print(".");
  }
  Serial.println();
  if (!!!client.connected()) {
    Serial.println("Failed to connect, going back to sleep");
  }

  Serial.print("Request resource: ");
  Serial.println(resource);
  client.print(String("GET ") + resource + APIKey + "&field1=" + temperatureC  + "&field2=" + digitalReader +
               " HTTP/1.1\r\n" +
               "Host: " + server + "\r\n" +
               "Connection: close\r\n\r\n");

  int timeout = 5 * 10; // 5 seconds
  while (!!!client.available() && (timeout-- > 0)) {
    delay(100);
  }
  if (!!!client.available()) {
    Serial.println("No response, going back to sleep");
  }
  while (client.available()) {
    Serial.write(client.read());
  }

  Serial.println("\nclosing connection");
  client.stop();
}
