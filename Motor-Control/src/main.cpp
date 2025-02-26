/*
 * @file main.cpp
 * @authors mia
 * @brief Main class for the motor control microcontroller
 * @version 0.1.0
 * @date 2024-03-10
 *
 * Copyright Deimo Elektronik GmbH (c) 2024
*/

#include "main.hpp"


Motor::MotorController motorcontroller; 
Wifi::WifiManager wifimanager;

void setup() 
{
  Serial.begin(SERIAL_BAUDRATE);
  
  // Start the wifi manager
  wifimanager.init();

  // Start the motor controller.
  motorcontroller.init();

  // Delete the loop task from the scheduler as we don't need it.
  vTaskDelete(NULL);
}

void loop() 
{
}
