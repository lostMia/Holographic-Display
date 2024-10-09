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

HTTPClient http_receive;
HTTPClient http_send;
uint16_t RPM_MOTOR = 0;
int target_speed = 0;

TaskHandle_t get_target_speed_task = NULL;
TaskHandle_t send_current_speed_task = NULL;


void setup() 
{
  Serial.begin(SERIAL_BAUDRATE);

  // Connect to the ESP32 access point
  WiFi.begin(AP_SSID, AP_PASSWORD);
  Serial.print("Connecting to main μC...");

  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");
  
  pinMode(MOTOR_PIN, INPUT);
  pinMode(39, INPUT);

  Serial.print("Creating connection for http_receive...");
  Serial.println(http_receive.begin(String(SERVER_DNS_NAME) + "/RPM"));

  // Task for receiving the target speed.
  xTaskCreate(
    get_target_speed,
    "Get Target Motor Speed",
    4096,
    NULL,
    2,
    &get_target_speed_task
  );
  
  delay(100);

  // Task for sending the current speed.
  xTaskCreate(
    send_current_speed,
    "Send Motor Speed",
    4096,
    NULL,
    1,
    &send_current_speed_task
  );
}


void loop() 
{
}


void get_target_speed(void *pvParameters)
{
  while (true)
  {
    if (WiFi.status() != WL_CONNECTED)
    {
      delay(1000);
      continue;
    }

    int httpCode;

    // Fetch desired motor speed from ESP32 server
    httpCode = http_receive.GET();

    if (httpCode <= 0) 
    {
      Serial.println("Error fetching motor speed");
      continue;
    }

    String payload = http_receive.getString();
    int target_speed_temp = payload.toInt();

    if (target_speed == target_speed_temp)
      continue;

    target_speed = target_speed_temp;
    Serial.println("New target speed: " + String(target_speed));
    
    set_motor_speed();

    vTaskDelay(GET_RPM_DELAY / portTICK_PERIOD_MS);  
  }
}


void send_current_speed(void *pvParameters)
{
  int httpCode;
  while (true)
  {    
    if (WiFi.status() != WL_CONNECTED)
    {
      Serial.println("Couldn't establish network connection!");
      delay(1000);
      continue;
    }

    if (!http_send.begin(String(SERVER_DNS_NAME) + String(SERVER_POST_SUFFIX)))
    {
      Serial.println("Couldn't establish http connection!");
      delay(1000);
      continue;
    }

    http_send.addHeader("Content-Type", "application/x-www-form-urlencoded");

    // Report actual motor speed back to the server
    // TODO: Get the RPM from the motor somehow.... i'm still waiting on timo for this.
    uint16_t actual_speed  = analogRead(39); // this will do for now
    String postData = "m1=" + String(actual_speed);

    Serial.println(postData);
    httpCode = http_send.POST(postData);
    http_send.end();
    
    if (httpCode <= 0)
      Serial.println("Couldn't post the current motor speed!");
    
    vTaskDelay(SEND_RPM_DELAY / portTICK_PERIOD_MS);  
  }
}


void set_motor_speed()
{
  analogWrite(MOTOR_PIN, target_speed);
}