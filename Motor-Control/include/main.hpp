/*
 * @file main.hpp
 * @authors Mia
 * @brief  
 * @version 0.1.0
 * @date 2024-03-10
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


void get_target_power(void *pvParameters);

void send_current_speed(void *pvParameters);

void get_last_pass_delay(void *pvParameters);
