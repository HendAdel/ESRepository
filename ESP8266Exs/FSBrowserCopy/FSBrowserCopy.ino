/*
  FSWebServer - Example WebServer with SPIFFS backend for esp8266
  Copyright (c) 2015 Hristo Gochkov. All rights reserved.
  This file is part of the ESP8266WebServer library for Arduino environment.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  upload the contents of the data folder with MkSPIFFS Tool ("ESP8266 Sketch Data Upload" in Tools menu in Arduino IDE)
  or you can upload the contents of a folder if you CD in that folder and run the following command:
  for file in `\ls -A1`; do curl -F "file=@$PWD/$file" esp8266fs.local/edit; done

  access the sample web page at http://esp8266fs.local
  edit the page by going to http://esp8266fs.local/edit
*/
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>

#define DBG_OUTPUT_PORT Serial

// Set the wifi network name and password
#ifndef STASSID
#define STASSID "M3mal Pro"
#define STAPSK  "M3mal.Pro"
#endif

const char* ssid = STASSID;
const char* password = STAPSK;
// Set the host to the esp8266 file system
const char* host = "esp8266fs";

// Set the ESP8266 Web Server to port 80
ESP8266WebServer server(80);

//holds the current upload
File fsUploadFile;

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
  DBG_OUTPUT_PORT.println("handleFileRead: " + path);
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

// Method to upload files.
void handleFileUpload() {
  // Check if it the edit page
  if (server.uri() != "/edit") {
    return;
  }

  //Set http upload varible to the server upload file
  HTTPUpload& upload = server.upload();

  // if the upload start and file name has not '/' before it add / before the name.
  if (upload.status == UPLOAD_FILE_START) {
    String filename = upload.filename;
    if (!filename.startsWith("/")) {
      filename = "/" + filename;
    }
    
    DBG_OUTPUT_PORT.print("handleFileUpload Name: "); DBG_OUTPUT_PORT.println(filename);
    // open the file in write mode with SPI flash file sytem
    fsUploadFile = SPIFFS.open(filename, "w");
    // Clear file name.
    filename = String();
    // Upload in write status
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    //DBG_OUTPUT_PORT.print("handleFileUpload Data: "); DBG_OUTPUT_PORT.println(upload.currentSize);
    // Write the file to File system!
    if (fsUploadFile) {
      fsUploadFile.write(upload.buf, upload.currentSize);
    }
    // If upload file end close the file
  } else if (upload.status == UPLOAD_FILE_END) {
    if (fsUploadFile) {
      fsUploadFile.close();
    }
    // Print the total file size
    DBG_OUTPUT_PORT.print("handleFileUpload Size: "); DBG_OUTPUT_PORT.println(upload.totalSize);
  }
}

// Delete exists file
void handleFileDelete() {

  // Check if the url hasn't parameters return bad args
  if (server.args() == 0) {
    return server.send(500, "text/plain", "BAD ARGS");
  }
  // else set the arguments to the path variable
  String path = server.arg(0);
  DBG_OUTPUT_PORT.println("handleFileDelete: " + path);

  // Check if the url has only the slash / return bad path
  if (path == "/") {
    return server.send(500, "text/plain", "BAD PATH");
  }
  // Check if the file exist in the path
  if (!SPIFFS.exists(path)) { // not exist return file not found
    return server.send(404, "text/plain", "FileNotFound");
  }
  // remove the file
  SPIFFS.remove(path); // file exist
  // TODO: try to add message the file removed successfully
  server.send(200, "text/plain", "");
  // Clear the path variable
  path = String();
}

// Create New file
void handleFileCreate() {
  // Check if the url hasn't parameters return bad args
  if (server.args() == 0) {
    return server.send(500, "text/plain", "BAD ARGS");
  }
  // else set the arguments to the path variable
  String path = server.arg(0);

  // Check if the url has only the slash / return bad path
  DBG_OUTPUT_PORT.println("handleFileCreate: " + path);
  if (path == "/") {
    return server.send(500, "text/plain", "BAD PATH");
  }
  // Check if the file exist in the path
  if (SPIFFS.exists(path)) { // file exist return FILE EXISTS
    return server.send(500, "text/plain", "FILE EXISTS");
  }
  // File not exist write the file
  File file = SPIFFS.open(path, "w");
  // if the created close it
  if (file) {
    file.close();
  } else { // Return Create failed
    return server.send(500, "text/plain", "CREATE FAILED");
  }
  // TODO: try to add message the file created successfully
  server.send(200, "text/plain", "");

  // Clear the path variable
  path = String();
}
// 
void handleFileList() {
  // Check if the url has parameter "dir"
  if (!server.hasArg("dir")) { // hasn't return bad args
    server.send(500, "text/plain", "BAD ARGS");
    return;
  }

  // Set the directory name to the path variable
  String path = server.arg("dir");
  DBG_OUTPUT_PORT.println("handleFileList: " + path);

  // Open the directory
  Dir dir = SPIFFS.openDir(path);

  // Clear path
  path = String();

  // Declear variable output and add the directory files in it with comma as separat
  String output = "[";
  while (dir.next()) {
    File entry = dir.openFile("r");
    if (output != "[") {
      output += ',';
    }
    
    bool isDir = false;

    // add file type and name to the output
    output += "{\"type\":\"";
    output += (isDir) ? "dir" : "file";
    output += "\",\"name\":\"";
    output += String(entry.name()).substring(1);
    output += "\"}";
    entry.close();
  }

  // Close the list
  output += "]";
  // Send the list.
  server.send(200, "text/json", output);
}

void setup(void) {
  // start seiral
  DBG_OUTPUT_PORT.begin(115200);
 
  DBG_OUTPUT_PORT.print("\n");
  DBG_OUTPUT_PORT.setDebugOutput(true);

  // start the flash file system
  SPIFFS.begin();
  {
    // Open the folder as Dir
    Dir dir = SPIFFS.openDir("/");
    // get the files names and sizes 
    while (dir.next()) {
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      DBG_OUTPUT_PORT.printf("FS File: %s, size: %s\n", fileName.c_str(), formatBytes(fileSize).c_str());
    }
    DBG_OUTPUT_PORT.printf("\n");
  }


  //WIFI INIT
  DBG_OUTPUT_PORT.printf("Connecting to %s\n", ssid);
  // Check if the wifi SSID not = the user defined ssid start the wifi with the user network name and password
  if (String(WiFi.SSID()) != String(ssid)) {
    WiFi.mode(WIFI_STA); // I don't know what is the wifif sta mode
    WiFi.begin(ssid, password);
  }

  // make a progress while connecting
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    DBG_OUTPUT_PORT.print(".");
  }
  DBG_OUTPUT_PORT.println("");
  DBG_OUTPUT_PORT.print("Connected! IP address: ");
  DBG_OUTPUT_PORT.println(WiFi.localIP());

  // start mdns for the file system 
  MDNS.begin(host);
  DBG_OUTPUT_PORT.print("Open http://");
  DBG_OUTPUT_PORT.print(host);
  DBG_OUTPUT_PORT.println(".local/edit to see the file browser");


  //SERVER INIT
  //list directory
  server.on("/list", HTTP_GET, handleFileList);
  //load editor - open the edit.html file in the browser -
  server.on("/edit", HTTP_GET, []() {
    if (!handleFileRead("/edit.htm")) {
      server.send(404, "text/plain", "FileNotFound");
    }
  });
  //create file - add each order to it's method -
  server.on("/edit", HTTP_PUT, handleFileCreate);
  //delete file 
  server.on("/edit", HTTP_DELETE, handleFileDelete);
  //first callback is called after the request has ended with all parsed arguments
  //second callback handles file uploads at that location
  server.on("/edit", HTTP_POST, []() {
    server.send(200, "text/plain", "");
  }, handleFileUpload);

  //called when the url is not defined here
  //use it to load content from SPIFFS
  server.onNotFound([]() {
    if (!handleFileRead(server.uri())) {
      server.send(404, "text/plain", "FileNotFound");
    }
  });

  //get heap status, analog input value and all GPIO statuses in one json call
  // didn't understand what is the heap, GPI and GPO
  server.on("/all", HTTP_GET, []() {
    String json = "{";
    json += "\"heap\":" + String(ESP.getFreeHeap());
    json += ", \"analog\":" + String(analogRead(A0)); // Read the analog input
    json += ", \"gpio\":" + String((uint32_t)(((GPI | GPO) & 0xFFFF) | ((GP16I & 0x01) << 16)));
    json += "}";
    // send the json
    server.send(200, "text/json", json);
    // clear json variable.
    json = String();
  });
  // start the server
  server.begin();
  DBG_OUTPUT_PORT.println("HTTP server started");

}

void loop(void) {
  // didn't understand what is the function for the mdns update.
  server.handleClient();
  
  MDNS.update();
}
