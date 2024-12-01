/*
 * @file wifimanager.hpp
 * @authors mia
 * @brief Defines a WifiManager class used for interacting with the WiFi component on the board.
 * @version 0.1.0
 * @date 2024-07.01
 *
 * Copyright Deimo Elektronik GmbH (c) 2024
*/

#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include "config.hpp"

#include "esp_wifi.h"

namespace Wireless
{

class WifiManager
{
private:
    IPAddress _IP;

    void _begin_AP();
    void _connect_AP();
public:
    void begin();
    IPAddress get_IP();
};

}
