
// ESP8266 Variable_Resistor_Sensor
const int sensorPin = A0;
const int ledPin = D0;
int sensorValue;

void setup() {
   // Initalize the ledPin pin as an output.
   pinMode(ledPin, OUTPUT);

}

void loop() {
   sensorValue = analogRead(sensorPin);
   digitalWrite(ledPin, HIGH);
   delay(sensorValue);
   digitalWrite(ledPin, LOW);
   delay(sensorValue);
}
