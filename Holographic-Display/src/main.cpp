
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
Wireless::WifiManager wifimanager;


void setup() 
{
  Serial.begin(SERIAL_BAUDRATE);

  // Delete the loop task from the scheduler, as we don't need it.
  vTaskDelete(NULL);
  
  // Start the wifi manager
  wifimanager.begin();
  
  // Start the webserver manager + rendering engine.
  server.begin();
}

void loop()
{
}
