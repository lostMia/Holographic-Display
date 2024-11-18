
/*
 * @file wifimanager.hpp
 * @authors mia
 * @brief Initializes and manages the WiFi connection, making sure that it's always up.
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


namespace Wifi
{

class WifiManager
{
private:
    TaskHandle_t _connect_to_wifi_task = NULL;

    static void connect(void *parameter);
public:
    void init();
};
    
}
