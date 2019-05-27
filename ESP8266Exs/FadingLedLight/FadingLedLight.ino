
//ESP8266 FadingLedLight

int ledPin = D0;
int i =0;

void setup() {
  // Initalize the ledPin pin as an output.
  pinMode(ledPin, OUTPUT);
}

void loop() {
  for(i = 0; i < 255; i++)
  {
    analogWrite(ledPin, i);
    delay(100);
  }
  for(i = 255; i > 0; i--)
  {
    analogWrite(ledPin, i);
    delay(100);
  }
}
