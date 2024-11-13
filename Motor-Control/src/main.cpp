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

uint16_t target_power = 0;
unsigned long delay_between_degrees_us = 0;

TaskHandle_t get_target_power_task = NULL;
TaskHandle_t send_current_speed_task = NULL;
TaskHandle_t get_last_pass_delay_task = NULL;

Servo motor;


void setup() 
{
  Serial.begin(SERIAL_BAUDRATE);
 
  // Connect to the ESP32 access point
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to main μC...");

  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");

  motor.setPeriodHertz(MOTOR_PWM_FREQUENCY);
  motor.attach(MOTOR_PIN, MOTOR_MIN, MOTOR_MAX);

  // Disable Watchdog because we want core 1 to be blocking all the time!
  disableCore1WDT();

  // Task for receiving the target power.
  // This task will be pinned to the first (0) core.
  xTaskCreatePinnedToCore(
    get_target_power,
    "Get Motor Power",
    4096,
    NULL,
    2,
    &get_target_power_task,
    0
  );

  // Task for sending the current speed.
  // This task will be pinned to the first (0) core.
  xTaskCreatePinnedToCore(
    send_current_speed,
    "Send Motor Speed",
    4096,
    NULL,
    1,
    &send_current_speed_task,
    0
  );

  // Task for sending the current speed.
  // This task will be pinned to the second (1) core.
  // Getting an accurate time here will require us to use the whole core doing constant polling.
  xTaskCreatePinnedToCore(
    get_last_pass_delay,
    "HAL Sensor Tracker",
    4096,
    NULL,
    1,
    &get_last_pass_delay_task,
    1
  );
}


void loop() 
{
}


void get_target_power(void *pvParameters)
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

    // Fetch desired motor power from ESP32 server
    httpCode = http_receive.GET();

    if (httpCode != HTTP_CODE_OK) 
    {
      Serial.println("Error fetching motor power");
      http_receive.end();
      vTaskDelay(DEFAULT_DELAY / portTICK_PERIOD_MS);  
      continue;
    }

    String payload = http_receive.getString();
    int target_power_temp = payload.toInt();

    if (target_power == target_power_temp)
    {
      http_receive.end();
      vTaskDelay(GET_RPM_DELAY / portTICK_PERIOD_MS);  
      continue;
    }

    target_power = target_power_temp;
    Serial.println("New target speed: " + String(target_power));
    
    motor.write(target_power);
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
    // TODO: get rid of this hardcoded stuff when we get an actual prototype we can test things out on..
    delay_between_degrees_us = 25000; // that's about 20 rps
    delay_between_degrees_us = target_power * 500; // todo: replace this with actual thing
    String postData = "m1=" + String(delay_between_degrees_us);

    httpCode = http_send.POST(postData);
    
    if (httpCode != HTTP_CODE_OK)
      Serial.println("Couldn't post the current motor speed!");

    http_send.end();
    
    vTaskDelay(SEND_RPM_DELAY / portTICK_PERIOD_MS);  
  }
}

void get_last_pass_delay(void *pvParameters)
{
  unsigned long delay_between_last_pass_us;
  unsigned long last_pass = micros();
  unsigned long current_time;

  while (true) 
  {
    // Wait for the motor to pass the first sensor.
    while (digitalRead(HAL_SENSOR1_PIN) == HIGH);
    
    current_time = micros();
    delay_between_last_pass_us = current_time - last_pass;
    delay_between_degrees_us = delay_between_degrees_us * 2 / 360;
    last_pass = current_time;
    
    Serial.println("Sensor passed! new delay:");
    Serial.println(delay_between_last_pass_us);

    while (digitalRead(HAL_SENSOR1_PIN) == LOW);
    
    // Wait for the motor to pass the second sensor.
    while (digitalRead(HAL_SENSOR2_PIN) == HIGH);
    
    current_time = micros();
    delay_between_last_pass_us = current_time - last_pass;
    delay_between_degrees_us = delay_between_degrees_us * 2 / 360;
    last_pass = current_time;
    
    Serial.println("Sensor passed! new delay:");
    Serial.println(delay_between_last_pass_us);

    while (digitalRead(HAL_SENSOR2_PIN) == LOW);
  }
}
