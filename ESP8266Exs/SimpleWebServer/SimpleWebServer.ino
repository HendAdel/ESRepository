#include <ESP8266WebServer.h> // The library for ESP8266WebServer
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <FS.h> // The library for 'SPIFFS'

// Set the wifi network name and password
#ifndef STASSID
#define STASSID "Focus"
#define STAPSK "Focus@Pro"
#endif
const char* ssid = STASSID;
const char* password = STAPSK;
// Set the host to the esp8266 file system
const char* host = "esp2866fs";

// Declare global variable
int counter = 1;
long mytime;

// Set the ESP8266 Web Server to port 80
ESP8266WebServer server(80);

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
  if (server.hasArg("download")) {
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
    path += "index.htm";
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
    server.streamFile(file, contentType);
    // close the file.
    file.close();
    return true;
  }
  return false;
}

// the callback function for the request.
void IncrementVariable(){
  /*// Check if it the increment request
  if (server.uri() != "/increment") {
    return;
  }
  delay(1000);*/

  // string array for hold the counter value
  char tempString[5];
  //counter++;
  // use printif because it add the value in the variable and print it.
  sprintf(tempString,"%d",counter);
  Serial.println("Counter increment: "+ counter);
  //send the counter value to the server
  server.send(200, "text/plain", tempString);
}

void setup() {
  // Start serial
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  //Start flash file system
  SPIFFS.begin();
  {
    // Open the direction
    Dir dir = SPIFFS.openDir("/");

    // Get the files names and sizes
    while (dir.next()){
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      Serial.printf("FS File: %s, Size: %s\n", fileName.c_str(), formatBytes(fileSize).c_str());
    }
    
  }
  mytime = millis();

  // WiFi Init.
  Serial.printf("Connecting to %s\n", ssid);

  // Check if the wifi ssid not equal the user defined ssid start the wifi with the user network settings
  if(String(WiFi.SSID()) != String(ssid)){
    WiFi.mode(WIFI_STA); // I don't know what is the wifif sta mode
    WiFi.begin(ssid, password);
  }

  // Make a progress while connecting
  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.print("Connected IP Address: ");
  Serial.println(WiFi.localIP());


  // Start mdns for the file system
  MDNS.begin(host);
  Serial.print("Open http://");
  Serial.print(host);
  Serial.println(".local to see the home page");

  //SERVER INIT
  //list directory
  //server.on("/", HTTP_GET, handleFileRead);
 
 
  //first callback is called after the request has ended with all parsed arguments
  //second callback handles file uploads at that location
 /* server.on("/edit", HTTP_POST, []() {
    server.send(200, "text/plain", "");
  }, handleFileUpload);*/

  //called when the url is not defined here
  //use it to load content from SPIFFS
  server.onNotFound([]() {
    if (!handleFileRead(server.uri())) {
      server.send(404, "text/plain", "FileNotFound");
    }
  });

   //call increment function
   Serial.print("in setup counter = " + counter);
  server.on("/increment", HTTP_GET, IncrementVariable);
  
 // Serial.println("Counter: "+ counter);
  server.on("/all", HTTP_GET, []() {
    String json = "{";
    json += "\"counter\":" + String(counter);
    json += "}";
    // send the json
    server.send(200, "text/json", json);
    // clear json variable.
    json = String();
  });
  // start the server
  server.begin();
  Serial.println("HTTP server started");
}
void loop() {

  //delay(1000); 
  if((millis() - mytime)>1000){
    counter++;
    mytime = millis();
  }
 
   // didn't understand what is the function for the mdns update.
  server.handleClient();
  
  MDNS.update();
}
