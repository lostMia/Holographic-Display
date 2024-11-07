#
#include <Arduino.h>

const int hallSensorPin = 27;
const int MotorPin = 26;

void setup() {
  pinMode(hallSensorPin, INPUT);
  pinMode(MotorPin, OUTPUT);

  // digitalWrite(MotorPin, HIGH);

  Serial.begin(115200);
}

void loop() {
  int sensorValue = analogRead(hallSensorPin);

  Serial.println("\ncurrent sensor value:");
  Serial.println(sensorValue);
  
  // digitalWrite(MotorPin, sensorValue == 0 ? HIGH : LOW);

  delay(50);
}