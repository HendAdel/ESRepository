
//ESP8266 Blink Sketch
int ledPin = D0;

void setup() {
  // Initalize the ledPin pin as an output.
  pinMode(ledPin, OUTPUT);
}

void loop() {
  // Onboard LED is on, and the LED connected to pin D0 is off.
  digitalWrite(ledPin, LOW);
  // Wait for a second
  delay(1000);
  // Onboard LED is off, and the LED connected to pin D0 is on.
  digitalWrite(ledPin, HIGH);
  // Wait for a second
  delay(1000);
}
