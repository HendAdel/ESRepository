
//ESP8266 ComputerInterfacing
int ledPin = D0;
int value;

void setup() {
  // start serial port
  Serial.begin(9600);
  // Initalize the ledPin pin as an output.
  pinMode(ledPin, OUTPUT);
}

void loop() {
  // read the serial and add it to value variable
   value = Serial.read();
   // check the value if 1 turn on the led
  if(value == '1') {digitalWrite(ledPin, HIGH);}
   // check the value if 0 turn off the led
  else if(value == '0') {digitalWrite(ledPin, LOW);}

}
