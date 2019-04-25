// ESP8266 Temprature_Sensor

const int sensorPin = A0;
int reading;
float voltage;
float temperatureC;

void setup() {
  // Start Serial port
  Serial.begin(9600);

}

void loop() {
  // Read the sensor data
  reading = analogRead(sensorPin);

  // calc the volt
  voltage = reading * (3300/1024);
  // print the volts value on the serial window
  Serial.print(voltage);
  Serial.println(" volts");

  // calc the Temperature from the volts
  temperatureC = (voltage - 500) / 100;
  
  // print the volts value on the serial window
  Serial.println("Temperature is: ");
  Serial.print(temperatureC);
  Serial.println(" degrees C");

  // wait 10 second
  delay(10000);

}
