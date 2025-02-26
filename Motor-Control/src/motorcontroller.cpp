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
  int http_code;

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
    http_code = motorcontroller->_http_receive.GET();

    if (http_code != HTTP_CODE_OK) 
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
    
    ledcWrite(MOTOR_PWM_CHANNEL, motorcontroller->_target_power);
    motorcontroller->_http_receive.end();

    vTaskDelay(GET_RPM_DELAY / portTICK_PERIOD_MS);  
  }
}

void MotorController::send_current_speed(void *parameter)
{
  MotorController *motorcontroller = (MotorController*)parameter;
  int http_code;

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
    
    unsigned long time_full_rotation_us = motorcontroller->_time_full_rotation_us;
    
    String post_data = "m1=" + String(time_full_rotation_us);

    http_code = motorcontroller->_http_send.POST(post_data);
    
    Serial.println(post_data);
    
    if (http_code != HTTP_CODE_OK)
      Serial.println("Couldn't post the current motor speed!");

    motorcontroller->_http_send.end();
    
    vTaskDelay(SEND_RPM_DELAY / portTICK_PERIOD_MS);  
  }
}

void IRAM_ATTR _motor_pulse_ISR(void* parameter) 
{
  MotorController *motorcontroller = (MotorController*)parameter;
  
  motorcontroller->handle_pulse();
}

void MotorController::init()
{
  pinMode(MOTOR_PULSE_FEEDBACK_PIN, INPUT_PULLUP);
  pinMode(MOTOR_PWM_SEND_PIN, OUTPUT);

  attachInterruptArg(digitalPinToInterrupt(MOTOR_PULSE_FEEDBACK_PIN), _motor_pulse_ISR, this, RISING);
  
  ledcSetup(MOTOR_PWM_CHANNEL, MOTOR_PWM_FREQUENCY, MOTOR_PWM_RESOLUTION);
  ledcAttachPin(MOTOR_PWM_SEND_PIN, MOTOR_PWM_CHANNEL);

  ledcWrite(MOTOR_PWM_CHANNEL, 0);
  
  while (true)
  {
    for (uint i = 100; i < 200; i++)
    {
      Serial.print("Full rotation time ms ->");
      Serial.println(this->_time_full_rotation_us);
      Serial.print("Pulse count -> ");
      Serial.println(this->_pulse_count);
      Serial.print("Loop Iteration ->");
      Serial.println(i);
      Serial.println("\n\n");
      ledcWrite(MOTOR_PWM_CHANNEL, i);
      delay(200);
    }

    for (uint i = 200; i > 100; i--)
    {
      Serial.print("Full rotation time ms ->");
      Serial.println(this->_time_full_rotation_us);
      Serial.print("Pulse count -> ");
      Serial.println(this->_pulse_count);
      Serial.print("Loop Iteration ->");
      Serial.println(i);
      Serial.println("\n\n");
      ledcWrite(MOTOR_PWM_CHANNEL, i);
      delay(200);
    }
  }

  // Task for receiving the target power.
  xTaskCreate(
    get_target_power,
    "Get Motor Power", 
    4096,
    this,
    1,
    &_get_target_power_task
  );

  // Task for sending the current speed.
  xTaskCreate(
    send_current_speed,
    "Send Motor Speed",
    4096,
    this,
    1,
    &_send_current_speed_task
  );
}

void MotorController::handle_pulse()
{
  // Set the time of the first pulse if it's not been set already.
  if (this->_pulse_count == 0)
    this->_time_first_pulse_us = micros();
  
  if (this->_pulse_count == MOTOR_PULSE_COUNT_FULL_ROTATION)
  {
    this->_pulse_count = 0;

    unsigned long current_time = micros();
    unsigned long time_difference = current_time - this->_time_first_pulse_us;

    // Even though the other core might be using this value, we don't really care since this action is atomic.
    this->_time_full_rotation_us = time_difference; 
  }
  else
    this->_pulse_count++;
}

}
