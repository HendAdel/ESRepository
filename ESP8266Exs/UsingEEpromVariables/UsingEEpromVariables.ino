#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h> // The library for ESP8266WebServer
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <FS.h> // The library for 'SPIFFS'

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
//const char* ssid     = "Focus";
//const char* password = "Focus@Pro";
const char* ssid     = "Mi Phone";
const char* password = "0100@Baba579";


// Replace with your unique Thing Speak WRITE API KEY
//const char* apiKey = "K7U14AOS7MM2OUUN";
//First channel api key
//const char* apiKey = "XN1BFT74XNJ18NR1";

const char* resource = "/update?api_key=";

// Thing Speak API server
const char* server = "api.thingspeak.com";

// Set the host to the esp8266 file system
const char* host = "esp2866fs";


// Set the ESP8266 Web Server to port 80
ESP8266WebServer espServer(80);


//format bytes size to known bytes units.
String formatBytes(size_t bytes) {
  if (bytes < 1024) {
    return String(bytes) + "B";
  } else if (bytes < (1024 * 1024)) {
    return String(bytes / 1024.0) + "KB";
  } else if (bytes < (1024 * 1024 * 1024)) {
    return String(bytes / 1024.0 / 1024.0) + "MB";
  } else {
    return String(bytes / 1024.0 / 1024.0 / 1024.0) + "GB";
  }
}


// check for the file extension to get the file type.
String getContentType(String filename) {
  if (espServer.hasArg("download")) {
    return "application/octet-stream";
  } else if (filename.endsWith(".htm")) {
    return "text/html";
  } else if (filename.endsWith(".html")) {
    return "text/html";
  } else if (filename.endsWith(".css")) {
    return "text/css";
  } else if (filename.endsWith(".js")) {
    return "application/javascript";
  } else if (filename.endsWith(".png")) {
    return "image/png";
  } else if (filename.endsWith(".gif")) {
    return "image/gif";
  } else if (filename.endsWith(".jpg")) {
    return "image/jpeg";
  } else if (filename.endsWith(".ico")) {
    return "image/x-icon";
  } else if (filename.endsWith(".xml")) {
    return "text/xml";
  } else if (filename.endsWith(".pdf")) {
    return "application/x-pdf";
  } else if (filename.endsWith(".zip")) {
    return "application/x-zip";
  } else if (filename.endsWith(".gz")) {
    return "application/x-gzip";
  }
  return "text/plain";
}

// Read the file
bool handleFileRead(String path) {
  Serial.println("handleFileRead: " + path);
  // If the path is the root add the index.htm to it.
  if (path.endsWith("/")) {
    path += "settings.htm";
  }
  // call the getContentType method and set the result to string varible.
  String contentType = getContentType(path);
  // Compress the file
  String pathWithGz = path + ".gz";

  //Check if the file exist on the flash file system zip or unzip.
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
    // Check again if the ziped file exist. I don't understand why and why zip it again!?
    if (SPIFFS.exists(pathWithGz)) {
      path += ".gz";
    }
    // Open the file in read mode
    File file = SPIFFS.open(path, "r");
    espServer.streamFile(file, contentType);
    // close the file.
    file.close();
    return true;
  }
  return false;
}

void setup() {

  Serial.begin(115200);

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

  //Start flash file system
  SPIFFS.begin();
  {
    // Open the direction
    Dir dir = SPIFFS.openDir("/");

    // Get the files names and sizes
    while (dir.next()) {
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      Serial.printf("FS File: %s, Size: %s\n", fileName.c_str(), formatBytes(fileSize).c_str());
    }

  }
  mytime = millis();
  initWifi();


  // Start mdns for the file system
  MDNS.begin(host);
  Serial.print("Open http://");
  Serial.print(host);
  Serial.println(".local to see the home page");
  espServer.onNotFound([]() {
    if (!handleFileRead(espServer.uri())) {
      espServer.send(404, "text/plain", "FileNotFound");
    }
  });

  //espServer.on("/",HTTP_GET, webpage);
  espServer.on("/", HTTP_POST, response);
  // start the server
  espServer.begin();
  Serial.println("HTTP server started");
}

void loop() {
  espServer.handleClient();

  MDNS.update();

  // make the request if the interval is valid
  if ((millis() - mytime) > (timeInterval * 1000)) {
    mytime = millis();
    Serial.println("timeInterval is: " + String(timeInterval));
    Serial.println("mytime is: " + String(millis() - mytime));
    digitalReader = digitalRead(digitalInput);
    makeHTTPRequest();
    // didn't understand what is the function for the mdns update.

  }
}

void response() {
  if (espServer.hasArg("apiKey") && (espServer.arg("apiKey").length() > 0)) { // TODO check that it's not longer than 31 characters
    Serial.print("User entered:\t");
    Serial.println(espServer.arg("apiKey"));
    APIKey = espServer.arg("apiKey");
    //    server.send(200, "text/html", "<html><body><h1>Successful</h1><a href='/'>Home</a></body></html>");
  }
  if (espServer.hasArg("interval") && (espServer.arg("interval").length() > 0)) { // TODO check that it's not longer than 31 characters
    Serial.print("User entered:\t");
    Serial.println(espServer.arg("interval"));
    //    timeInterval = int(espServer.arg("interval"));
    //    server.send(200, "text/html", "<html><body><h1>Successful</h1><a href='/'>Home</a></body></html>");
  }
  //else {
  //    server.send(400, "text/html", "<html><body><h1>HTTP Error 400</h1><p>Bad request. Please enter a value.</p></body></html>");
  //  }
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
