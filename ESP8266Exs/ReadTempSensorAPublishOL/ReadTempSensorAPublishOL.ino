// ESP8266 Temprature_Sensor

#include <ESP8266WiFi.h>

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
const char* apiKey = "K7U14AOS7MM2OUUN";
//First channel api key
//const char* apiKey = "XN1BFT74XNJ18NR1";

const char* resource = "/update?api_key=";

// Thing Speak API server
const char* server = "api.thingspeak.com";


// Temporary variables
static char temperatureTemp[7];
static char humidityTemp[7];


void setup() {
  // Start Serial port
  Serial.begin(9600);
  pinMode(digitalInput, INPUT);

  initWifi();
}

void loop() {

  digitalReader = digitalRead(digitalInput);
  makeHTTPRequest();
}

// Establish a Wi-Fi connection with your router
void initWifi() {
  Serial.print("Connecting to: ");
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
  client.print(String("GET ") + resource + apiKey + "&field1=" + temperatureC  + "&field2=" + digitalReader +
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
