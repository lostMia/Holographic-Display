/*
 * @file webserver.hpp
 * @authors Mia
 * @brief Defines a WebServer class for interacting with the webserver.
 * @version 0.1.0
 * @date 2024-06-27
 *
 * Copyright Deimo Elektronik GmbH (c) 2024
*/

#pragma once

#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <LittleFS.h>
#include <iomanip>
#include <string>
#include <iostream>
#include <sstream>
#include "config.hpp"
#include "esp_log.h"
#include "esp_task_wdt.h"
#include "Rendering/rendering.hpp"

#ifdef OTA_FIRMWARE
#define ELEGANTOTA_USE_ASYNC_WEBSERVER 1

#include <ElegantOTA.h>
#endif


namespace Wireless
{

#define MSS 1500

class WebServer
{
private:
    AsyncWebServer _server;
    Rendering::Renderer* _renderer;
    
    uint16_t _target_power = 0;
    bool _motor_enabled = true;
    uint8_t _led_brightness = 50;
    uint16_t _current_RPM = 0;
    uint8_t _next_upload_print = 0;
    bool _can_upload = true;
    bool _dmo_mode = true;
    uint8_t _frame_buffer[IMAGE_SIZE_BYTES + MSS];
    uint16_t _frame_buffer_index = 0;
    uint8_t _frame_counter = 0;

    TaskHandle_t _OTA_loop_task = NULL;
    

#ifdef OTA_FIRMWARE
    void _begin_OTA();
#endif

#ifdef MDNS_HOSTNAME
    bool _begin_mDNS();
#endif
    
    void _setup_webserver_tree();
    
    void _handle_input(const AsyncWebParameter* parameter);
    String _format_bytes(const size_t bytes);
public:

    WebServer(uint16_t port, Rendering::Renderer *renderer);
    void begin();
};

}