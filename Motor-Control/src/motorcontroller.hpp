
/*
 * @file motorontroller.hpp
 * @authors Mia
 * @brief Motor Controller manager class
 * @version 0.1.0
 * @date 2024-11-18
 *
 * Copyright Deimo Elektronik GmbH (c) 2024
*/

#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ESP32Servo.h> 

#include "credentials.hpp"
#include "config.hpp"

namespace Motor 
{

class MotorController 
{
private:
    HTTPClient _http_receive;
    HTTPClient _http_send;

    uint16_t _target_power = 0;
    unsigned long _delay_between_last_pass_us = 0;

    TaskHandle_t _get_target_power_task = NULL;
    TaskHandle_t _send_current_speed_task = NULL;
    TaskHandle_t _get_last_pass_delay_task = NULL;

    Servo _motor;
    
    static void get_target_power(void *parameter);
    static void send_current_speed(void *parameter);
    static void get_last_pass_delay(void *parameter);
    void calculate_new_delay(unsigned long* plast_pass);
public:
    void init();
};

}


