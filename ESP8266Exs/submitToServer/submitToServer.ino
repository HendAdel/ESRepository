#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

#ifndef STASSID
#define STASSID "Focus"
#define STAPSK  "Focus@Pro"
#endif

/*
  #ifndef STASSID
  #define STASSID "Mi Phone"
  #define STAPSK  "0100@Baba579"
  #endif
*/
const char* ssid = STASSID;
const char* password = STAPSK;

ESP8266WebServer server(80);

//Settings page
void handleSettings() {
  Serial.println(" in handleSettings");
  String msg;

  if (server.hasArg("apiKey") && server.hasArg("interval")) {
    Serial.println("apiKey" + server.arg("apiKey"));
    Serial.println("interval" + server.arg("interval") );
    if (server.arg("apiKey") != "" &&  server.arg("interval") != "") {
      server.sendHeader("Location", "/");
      server.send(301);
      Serial.println("settings set in Successful");
      return;
    }
    msg = "You did not enter any settings";
    //Serial.println("Log in Failed");
  }
  String content = "<html><head><style>body {background-image: url(http://kreativo.se/backlogin.jpg);";
  content += "font-family: 'Helvetica Neue', Helvetica, Arial;padding-top: 20px;}";
  content += ".container {width: 406px; max-width: 406px; margin: 0 auto;}";
  content += "#settings { padding: 0px 25px 25px; background: #fff; box-shadow: 0px 0px 0px 5px rgba( 255,255,255,0.4 ),";
  content += "0px 4px 20px rgba( 0,0,0,0.33 );-moz-border-radius: 5px;-webkit-border-radius: 5px;border-radius: 5px;";
  content += "display: table; position: static;}";
  content += "#settings .header {margin-bottom: 20px;}";
  content += "#settings .header h3 {color: #333333; font-size: 24px; font-weight: bold; margin-bottom: 5px;}";
  content += "#settings .header p {color: #8f8f8f; font-size: 14px; font-weight: 300; }";
  content += "#settings .sep { height: 1px; background: #e8e8e8; width: 406px; margin: 0px -25px; }";
  content += "#settings .inputs { margin-top: 25px;}";
  content += "#settings .inputs label { color: #8f8f8f; font-size: 12px; font-weight: 300; letter-spacing: 1px; margin-bottom: 7px; display: block; }";
  content += "input::-webkit-input-placeholder { color:    #b5b5b5; }";
  content += "input:-moz-placeholder { color:    #b5b5b5; }";
  content += "#settings .inputs input[type=apiKey], input[type=interval] { background: #f5f5f5;  font-size: 0.8rem;";
  content += "-moz-border-radius: 3px; -webkit-border-radius: 3px; border-radius: 3px; border: none; padding: 13px 10px;";
  content += "width: 330px; margin-bottom: 20px; box-shadow: inset 0px 2px 3px rgba( 0,0,0,0.1 ); clear: both; }";
  content += "#settings .inputs input[type=apiKey]:focus, input[type=interval]:focus { background: #fff;";
  content += "box-shadow: 0px 0px 0px 3px #fff38e, inset 0px 2px 3px rgba( 0,0,0,0.2 ), 0px 5px 5px rgba( 0,0,0,0.15 ); outline: none;  }";
  content += "#settings .inputs .checkboxy { display: block; position: static; height: 25px; margin-top: 10px; clear: both; }";
  content += "#settings .inputs input[type=checkbox] { float: left; margin-right: 10px; margin-top: 3px; }";
  content += "#settings .inputs label.terms { float: left; font-size: 14px; font-style: italic; }";
  content += "#settings .inputs #submit { width: 100%; margin-top: 20px; padding: 15px 0; color: #fff;";
  content += "font-size: 14px; font-weight: 500; letter-spacing: 1px; text-align: center; text-decoration: none;";
  content += "background: -moz-linear-gradient( top, #b9c5dd 0%, #a4b0cb);";
  content += "background: -webkit-gradient( linear, left top, left bottom,  from(#b9c5dd), to(#a4b0cb));";
  content += " -moz-border-radius: 5px;  -webkit-border-radius: 5px; border-radius: 5px; border: 1px solid #737b8d;";
  content += " -moz-box-shadow: 0px 5px 5px rgba(000,000,000,0.1), inset 0px 1px 0px rgba(255,255,255,0.5);";
  content += " -webkit-box-shadow: 0px 5px 5px rgba(000,000,000,0.1), inset 0px 1px 0px rgba(255,255,255,0.5);";
  content += "box-shadow: 0px 5px 5px rgba(000,000,000,0.1), inset 0px 1px 0px rgba(255,255,255,0.5);";
  content += "text-shadow: 0px 1px 3px rgba(000,000,000,0.3), 0px 0px 0px rgba(255,255,255,0);";
  content += "display: table; position: static; clear: both;}";
  content += "#settings .inputs #submit:hover { background: -moz-linear-gradient( top, #a4b0cb 0%, #b9c5dd);";
  content += "background: -webkit-gradient( linear, left top, left bottom,  from(#a4b0cb), to(#b9c5dd)); }";
  content += "</style></head><body><div class='container'><form action='/' method='POST'><div class='header'><h3>Control Settings</h3><br>";
  content += "</div><div class='sep'></div><div class='inputs'><input id='apiKey' name='apiKey' type='apiKey' placeholder='API Key' autofocus /><br>";
  content += "<input id='interval' name='interval' type='interval' placeholder='Interval' /><br>";
  content += "<div class='checkboxy'>";
  content += "<input id='enable' name='cecky' id='checky' value='1' type='checkbox' /><label class='terms'>Enable</label></div>";
  content += "<input id='submit' type='submit' name='SUBMIT' value='Submit'></div></form>" + msg + "<br>";
  content += "</body></html>";
  server.send(200, "text/html", content);
}

//no need authentication
void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void setup(void) {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());


  // server.on("/", handleRoot);
  server.on("/", handleSettings);
  //  server.on("/inline", []() {
  //    server.send(200, "text/plain", "this works without need of authentication");
  //  });

  server.onNotFound(handleNotFound);
  //here the list of headers to be recorded
  const char * headerkeys[] = {"User-Agent", "Cookie"} ;
  size_t headerkeyssize = sizeof(headerkeys) / sizeof(char*);
  //ask server to track these headers
  server.collectHeaders(headerkeys, headerkeyssize);
  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
}
