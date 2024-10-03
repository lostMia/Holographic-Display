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

HTTPClient http;
uint16_t RPM_MOTOR = 0;
int httpCode;
int target_speed;

TaskHandle_t send_speed_task = NULL;


void setup() 
{
  Serial.begin(SERIAL_BAUDRATE);

  // Connect to the ESP32 access point
  WiFi.begin(AP_SSID, AP_PASSWORD);
  Serial.println("Connecting to main μC...");

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.println(".");
  }
  Serial.println("Connected to WiFi");
  
  pinMode(MOTOR_PIN, OUTPUT);

  http.begin(String(SERVER_DNS_NAME) + "/RPM");

  // Create the first task to blink LED1 every 500 ms
  xTaskCreate(
    send_current_speed,        // Task function
    "Send Motor Speed", // Name of the task
    1000,         // Stack size in words
    NULL,         // Task input parameter
    1,            // Priority of the task
    &send_speed_task  // Task handle
  );
}


void loop() 
{
  if (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    return;
  }
  
  // If the target speed hasn't changed, simply keep waiting.
  if (!get_target_speed())
  {
    delay(FETCH_RPM_DELAY);
    return;
  }
  
  // TODO: Do some really fancy conversion here.
  // and make the motor change it's speed i guess.
  // for now just map the value directly to the analog output.
  analogWrite(MOTOR_PIN, map(target_speed, 0, 255, 0, 255));

  delay(2000);
}


bool get_target_speed()
{
  // Fetch desired motor speed from ESP32 server
  httpCode = http.GET();

  if (httpCode == 0) 
  {
    Serial.println("Error fetching motor speed");
    return false;
  }

  String payload = http.getString();
  int target_speed_temp = payload.toInt();
  
  if (target_speed == target_speed_temp)
    return false;

  target_speed = target_speed_temp;
  Serial.println("New target speed: " + String(target_speed));
  return true;
}


void send_current_speed(void *pvParameters)
{
  while (true)
  {
    // Report actual motor speed back to the server
    // TODO: Get the RPM from the motor somehow.... i'm still waiting on timo for this.
    uint16_t actual_speed  = 0;
    String postData = "m" + String(actual_speed);
    httpCode = http.POST(postData);
    
    if (httpCode == 0)
      Serial.println("Couldn't post the current motor speed!");
    
    delay(SEND_RPM_DELAY);
  }
}