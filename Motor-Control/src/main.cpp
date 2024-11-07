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
#include <ESP32Servo.h> 

HTTPClient http_receive;
HTTPClient http_send;
uint16_t actual_speed = 0;
uint16_t target_speed = 0;

TaskHandle_t get_target_speed_task = NULL;
TaskHandle_t send_current_speed_task = NULL;
TaskHandle_t count_motor_passes_task = NULL;

Servo motor;


void setup() 
{
  Serial.begin(SERIAL_BAUDRATE);
 
  // Connect to the ESP32 access point
  // WiFi.begin(AP_SSID, AP_PASSWORD);
  Serial.print("Connecting to main μC...");

  // while (WiFi.status() != WL_CONNECTED) 
  // {
  //   delay(500);
  //   Serial.print(".");
  // }
  // Serial.println("Connected to WiFi");
  // 
  // motor.setPeriodHertz(MOTOR_PWM_FREQUENCY);
  // motor.attach(MOTOR_PIN, MOTOR_MIN, MOTOR_MAX);
  //
  // // Task for receiving the target speed.
  // xTaskCreate(
  //   get_target_speed,
  //   "Get Target Motor Speed",
  //   4096,
  //   NULL,
  //   2,
  //   &get_target_speed_task
  // );
  // 
  // delay(50);
  //
  // // Task for sending the current speed.
  // xTaskCreate(
  //   send_current_speed,
  //   "Send Motor Speed",
  //   4096,
  //   NULL,
  //   1,
  //   &send_current_speed_task
  // );

  // Task for sending the current speed.
  xTaskCreate(
    count_motor_passes,
    "Keeps track of the amount of times, the motor has passed the HALL sensor",
    4096,
    NULL,
    1,
    &count_motor_passes_task
  );
}


void loop() 
{
}


void get_target_speed(void *pvParameters)
{
  int httpCode;

  while (true)
  {
    if (WiFi.status() != WL_CONNECTED)
    {
      Serial.println("Couldn't establish network connection!");
      vTaskDelay(DEFAULT_DELAY / portTICK_PERIOD_MS);  
      continue;
    }

    if (!http_receive.begin(String(SERVER_DNS_NAME) + String(SERVER_GET_SUFFIX)))
    {
      Serial.println("Couldn't establish http connection for getting the motor value!");
      vTaskDelay(DEFAULT_DELAY / portTICK_PERIOD_MS);  
      continue;
    }

    // Fetch desired motor speed from ESP32 server
    httpCode = http_receive.GET();

    if (httpCode != HTTP_CODE_OK) 
    {
      Serial.println("Error fetching motor speed");
      http_receive.end();
      vTaskDelay(DEFAULT_DELAY / portTICK_PERIOD_MS);  
      continue;
    }

    String payload = http_receive.getString();
    int target_speed_temp = payload.toInt();

    if (target_speed == target_speed_temp)
    {
      http_receive.end();
      vTaskDelay(GET_RPM_DELAY / portTICK_PERIOD_MS);  
      continue;
    }

    target_speed = target_speed_temp;
    Serial.println("New target speed: " + String(target_speed));
    
    set_motor_speed();
    http_receive.end();
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
      vTaskDelay(DEFAULT_DELAY / portTICK_PERIOD_MS);  
      continue;
    }

    if (!http_send.begin(String(SERVER_DNS_NAME) + String(SERVER_POST_SUFFIX)))
    {
      Serial.println("Couldn't establish http connection for sending the motor value!");
      vTaskDelay(DEFAULT_DELAY / portTICK_PERIOD_MS);  
      continue;
    }

    http_send.addHeader("Content-Type", "application/x-www-form-urlencoded");

    // Report actual motor speed back to the server
    // TODO: Get the RPM from the motor somehow.... i'm still waiting on timo for this.
    // actual_speed += (target_speed - actual_speed) / 20; // get rid of this
    String postData = "m1=" + String(actual_speed);

    Serial.println(postData);
    httpCode = http_send.POST(postData);
    
    if (httpCode != HTTP_CODE_OK)
      Serial.println("Couldn't post the current motor speed!");

    http_send.end();
    
    vTaskDelay(SEND_RPM_DELAY / portTICK_PERIOD_MS);  
  }
}


void count_motor_passes(void *pvParameters)
{
  int motor_passes = 0;

  Serial.println("started thread......");
  
  while (true) 
  {
    while (digitalRead(HAL_SENSOR_PIN) == HIGH) 
    {

      vTaskDelay(1 / portTICK_PERIOD_MS);  
    }

    while (digitalRead(HAL_SENSOR_PIN) == LOW)
    {

      vTaskDelay(1 / portTICK_PERIOD_MS);  
    }



    motor_passes += 1;
    Serial.println(motor_passes);
  }
}


void set_motor_speed()
{
  int motor_value = target_speed; // ToDo: improve this 
  motor.write(motor_value);
}