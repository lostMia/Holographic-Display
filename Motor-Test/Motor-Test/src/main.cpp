#include <Arduino.h>
#include <ESP32Servo.h> 

Servo motor_servo;  // create servo object to control a servo

#define SERVO_PIN 17 // ESP32 pin GPIO26 connected to servo motor

void setup() {
  Serial.begin(115200);

  motor_servo.setPeriodHertz(50);// Standard 50hz servo
  motor_servo.attach(SERVO_PIN, 1000, 5000);

}

void loop() 
{
 // Check if data is available on the Serial interface
  if (Serial.available()) {
    // Read the input value from the Serial monitor
    int inputPWMValue = Serial.parseInt();

    // Make sure the value is between 0 and 255
    if (inputPWMValue >= 0 && inputPWMValue <= 255) {
      // Output the PWM value on the output pin
      motor_servo.write(inputPWMValue);
      
      // Print confirmation to the Serial monitor
      Serial.print("PWM output set to: ");
      Serial.println(inputPWMValue);
    } else {
      Serial.println("Please enter a valid PWM value between 0 and 255.");
    }
  }
}
