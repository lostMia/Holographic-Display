
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
#include <vector>
#include "credentials.hpp"
#include "config.hpp"

namespace Motor 
{

void IRAM_ATTR _motor_pulse_ISR();

class MotorController 
{
private:
    HTTPClient _http_receive;
    HTTPClient _http_send;

    uint16_t _target_power = 0;
    unsigned long _current_delay_per_pulse_us = 0;
    unsigned long _time_last_pulse_us = 0;
    std::vector<unsigned long> _last_delays_us;

    TaskHandle_t _get_target_power_task = NULL;
    TaskHandle_t _send_current_speed_task = NULL;

    static void get_target_power(void *parameter);
    static void send_current_speed(void *parameter);

#ifdef USE_AVERAGED_DELAY
    unsigned long _get_average_pulse();
#endif
public:
    void init();
    void handle_pulse();
};

}


