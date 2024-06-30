 /*
 * @file Main.hpp
 * @authors Mia
 * @brief 
 * @version 0.1.0
 * @date 2024-05-15
 *
 * Copyright Deimo Elektronik GmbH (c) 2024
*/

#pragma once


#include <Arduino.h>
#include "credentials.hpp"
#include "Webpage/webserver.hpp"

#ifndef WIFI_PASSWORD
    #error "The Variable "WIFI_PASSWORD" is not set! You can do this by creating a credentials.hpp file in the include directory and defining the variable there!"
#endif

#ifndef WIFI_SSID
    #error "The Variable "WIFI_SSID" is not set! You can do this by creating a credentials.hpp file in the include directory and defining the variable there!"
#endif

#define SERIAL_BAUDRATE 115200

#define WEBSERVER_PORT 80