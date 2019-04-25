
// Example_6 Fading_led_light
const int ledPin = 11;
int i =0;

void setup() {
  // put your setup code here, to run once:
  pinMode(ledPin, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  for(i = 0; i < 255; i++)
  {
    analogWrite(ledPin, i);
    delay(10);
  }
  for(i = 255; i > 0; i--)
  {
    analogWrite(ledPin, i);
    delay(10);
  }
  
}
