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
#include <driver/spi_master.h>
#include "credentials.hpp"
#include "config.hpp"
#include "Wireless/webserver.hpp"
#include "Wireless/wifimanager.hpp"
#include "Rendering/rendering.hpp"
#include "esp_log.h"

Rendering::Renderer renderer;
Wireless::WebServer server(WEBSERVER_PORT, &renderer);
Wireless::WifiManager wifimanager;