
//Example7_Temprature_Sensor

const int sensorPin = A0;
int reading;
float voltage;
float temperatureC;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

}

void loop() {
  // put your main code here, to run repeatedly:
  reading = analogRead(sensorPin);
  voltage = reading * 5.0/1024;
  Serial.print(voltage);
  Serial.println(" volts");
  temperatureC = (voltage - 0.5) * 100;
  Serial.println("Temperature is: ");
  Serial.print(temperatureC);
  Serial.println(" degrees C");
  delay(10000);

}
