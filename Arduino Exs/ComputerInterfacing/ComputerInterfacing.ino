
//Example_9 ComputerInterfacing
int ledPin = 13;
int value;

void setup() {
  
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);

}

void loop() {
  
  // put your main code here, to run repeatedly:
  value = Serial.read();
  if(value == '1') {digitalWrite(ledPin, HIGH);}
  else if(value == '0') {digitalWrite(ledPin, LOW);}
  
}
