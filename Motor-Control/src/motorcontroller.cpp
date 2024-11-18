/*
 * @file motorcontroller.cpp
 * @authors Mia
 * @brief Motor Controller manager class
 * @version 0.1.0
 * @date 2024-11-18
 *
 * Copyright Deimo Elektronik GmbH (c) 2024
*/

#include "motorcontroller.hpp"


namespace Motor
{

void MotorController::get_target_power(void *parameter)
{
  MotorController *motorcontroller = (MotorController*)parameter;
  int httpCode;

  while (true)
  {
    if (WiFi.status() != WL_CONNECTED)
    {
      Serial.println("Couldn't establish network connection!");
      vTaskDelay(DEFAULT_DELAY / portTICK_PERIOD_MS);  
      continue;
    }

    if (!motorcontroller->_http_receive.begin(String(SERVER_DNS_NAME) + String(SERVER_GET_SUFFIX)))
    {
      Serial.println("Couldn't establish http connection for getting the motor value!");
      vTaskDelay(DEFAULT_DELAY / portTICK_PERIOD_MS);  
      continue;
    }

    // Fetch desired motor power from ESP32 server
    httpCode = motorcontroller->_http_receive.GET();

    if (httpCode != HTTP_CODE_OK) 
    {
      Serial.println("Error fetching motor power");
      motorcontroller->_http_receive.end();
      vTaskDelay(DEFAULT_DELAY / portTICK_PERIOD_MS);  
      continue;
    }

    String payload = motorcontroller->_http_receive.getString();
    int target_power_temp = payload.toInt();

    if (motorcontroller->_target_power == target_power_temp)
    {
      motorcontroller->_http_receive.end();
      vTaskDelay(GET_RPM_DELAY / portTICK_PERIOD_MS);  
      continue;
    }

    motorcontroller->_target_power = target_power_temp;
    Serial.println("New target speed: " + String(motorcontroller->_target_power));
    
    // motor.writeMicroseconds(target_power);
    // motor.write(target_power);
    motorcontroller->_http_receive.end();

    vTaskDelay(GET_RPM_DELAY / portTICK_PERIOD_MS);  
  }
}

void MotorController::send_current_speed(void *parameter)
{
  MotorController *motorcontroller = (MotorController*)parameter;
  int httpCode;

  while (true)
  {
    if (WiFi.status() != WL_CONNECTED)
    {
      Serial.println("Couldn't establish network connection!");
      vTaskDelay(DEFAULT_DELAY / portTICK_PERIOD_MS);  
      continue;
    }

    if (!motorcontroller->_http_send.begin(String(SERVER_DNS_NAME) + String(SERVER_POST_SUFFIX)))
    {
      Serial.println("Couldn't establish http connection for sending the motor value!");
      vTaskDelay(DEFAULT_DELAY / portTICK_PERIOD_MS);  
      continue;
    }

    motorcontroller->_http_send.addHeader("Content-Type", "application/x-www-form-urlencoded");

    String postData = "m1=" + String(motorcontroller->_delay_between_last_pass_us);

    httpCode = motorcontroller->_http_send.POST(postData);
    
    Serial.println(postData);
    
    if (httpCode != HTTP_CODE_OK)
      Serial.println("Couldn't post the current motor speed!");

    motorcontroller->_http_send.end();
    
    vTaskDelay(SEND_RPM_DELAY / portTICK_PERIOD_MS);  
  }
}

void MotorController::get_last_pass_delay(void *parameter)
{
  MotorController *motorcontroller = (MotorController*)parameter;
  unsigned long last_pass = micros();

  while (true) 
  {
    // wait for the motor to pass the first sensor.
    while (analogRead(HAL_SENSOR1_PIN) == 4095);
    
    motorcontroller->calculate_new_delay(&last_pass);

    // Wait for the motor to pass the second sensor.
    // while (analogRead(HAL_SENSOR1_PIN) != 4095);
    while (analogRead(HAL_SENSOR2_PIN) == 4095); // replace top with this when having 2 sensors connected.
    
    motorcontroller->calculate_new_delay(&last_pass);
  }
}

void MotorController::calculate_new_delay(unsigned long* plast_pass)
{
  unsigned long current_time;

  current_time = micros();
  _delay_between_last_pass_us = current_time - *plast_pass;
  *plast_pass = current_time;
  
  Serial.println(_delay_between_last_pass_us);
}


void MotorController::init()
{
  _motor.setPeriodHertz(MOTOR_PWM_FREQUENCY);
  _motor.attach(MOTOR_PIN, MOTOR_MIN, MOTOR_MAX);
  
  // Disable Watchdog because we want core 1 to be blocking all the time!
  disableCore0WDT();

  pinMode(HAL_SENSOR1_PIN, INPUT);
  pinMode(HAL_SENSOR2_PIN, INPUT);

  // Task for receiving the target power.
  // This task will be pinned to the first (0) core.
  xTaskCreatePinnedToCore(
    get_target_power,
    "Get Motor Power",
    4096,
    this,
    1,
    &_get_target_power_task,
    1
  );
  
  // Task for sending the current speed.
  // This task will be pinned to the first (0) core.
  xTaskCreatePinnedToCore(
    send_current_speed,
    "Send Motor Speed",
    4096,
    this,
    1,
    &_send_current_speed_task,
    1
  );
  
  delay(100);

  // Task for sending the current speed.
  // This task will be pinned to the second (1) core.
  // Getting an accurate time here will require us to use the whole core doing constant polling.
  xTaskCreatePinnedToCore(
    get_last_pass_delay,
    "HAL Sensor Tracker",
    4096,
    this,
    1,
    &_get_last_pass_delay_task,
    0
  ); 
}

}
