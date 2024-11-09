
/*
 * @file main.cpp
 * @authors mia
 * @brief Main class for getting values from the internet and hosting a webserver.
 * @version 0.1.0
 * @date 2024-05-15
 *
 * Copyright Deimo Elektronik GmbH (c) 2024
*/

#include "main.hpp"

Rendering::Renderer renderer;
Wireless::WebServer server(WEBSERVER_PORT, &renderer);
Wireless::WifiManager wifi_manager;


void setup() 
{
  Serial.begin(SERIAL_BAUDRATE);
  
  wifi_manager.begin();
  server.begin();
}

void loop()
{
#ifdef OTA_FIRMWARE
  ElegantOTA.loop();
#endif
}
