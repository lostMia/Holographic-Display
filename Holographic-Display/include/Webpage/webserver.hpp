/*
 * @file webserver.hpp
 * @authors Mia
 * @brief Defines a Server class for interacting with the webserver.
 * @version 0.1.0
 * @date 2024-06-27
 *
 * Copyright Deimo Elektronik GmbH (c) 2024
*/

#pragma once


#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <SPIFFS.h>

namespace Web
{

class Server
{
private:
    AsyncWebServer _server;
        
    void _add_to_json_string(String* string, String* parameter, float* value);
public:
    Server(uint16_t port);

    void begin();
};

}