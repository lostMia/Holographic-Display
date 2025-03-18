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
    unsigned long time_between_pulses_us;

    // If we didn't get a pulse in the maximum allowed time then consider the motor stopped.
    if ((micros() - motorcontroller->_time_last_pulse_us) > LAST_PULSE_MAX_DELAY_US)
      time_between_pulses_us = 0;
    else
      time_between_pulses_us = motorcontroller->_get_average_pulse();
    
    String post_data = "m1=" + String(time_between_pulses_us);
    http_code = motorcontroller->_http_send.POST(post_data);
    Serial.println(post_data);
    
    if (http_code != HTTP_CODE_OK)
      Serial.println("Couldn't post the current motor speed!");

    motorcontroller->_http_send.end();
    
    vTaskDelay(SEND_RPM_DELAY / portTICK_PERIOD_MS);  
  }

}

unsigned long MotorController::_get_average_pulse()
{
  if (_last_delays_us.empty()) 
    return 0;

  unsigned long sum = 0;

  for (unsigned long delay : _last_delays_us)
    sum += delay;
    
  return sum / _last_delays_us.size();
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
  unsigned long current_delay_per_pulse_us = 0; 
  unsigned long current_time = micros();
  
  this->_current_delay_per_pulse_us = current_time - this->_time_last_pulse_us;
  
  if (_last_delays_us.size() == LAST_PULSES_TO_AVERAGE)
    _last_delays_us.erase(_last_delays_us.begin());

  _last_delays_us.push_back(this->_current_delay_per_pulse_us);
  
  this->_time_last_pulse_us = current_time;
}

}
