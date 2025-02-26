/*
 * @file wifimanager.hpp
 * @authors mia
 * @brief Initializes and manages the WiFi connection, making sure that it's always up.
 * @version 0.1.0
 * @date 2024-11-18
 *
 * Copyright Deimo Elektronik GmbH (c) 2024
*/

#include "wifimanager.hpp"


namespace Wifi
{
    
void WifiManager::init()
{
  // Task for making sure we're connected to the WiFi.
  xTaskCreate(
    connect,
    "Connect to Wifi",
    32768,
    this,
    1,
    &_connect_to_wifi_task
  );
}

void WifiManager::connect(void *parameter)
{
  WifiManager* wifimanager = (WifiManager*)parameter;

  while (true)
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      vTaskDelay(DEFAULT_DELAY / portTICK_PERIOD_MS);
      continue;
    }

    // Connect to the ESP32 access point.
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    Serial.print("Connecting to main μC...");

    while (WiFi.status() != WL_CONNECTED) 
    {
      Serial.print(".");
      vTaskDelay(DEFAULT_DELAY / portTICK_PERIOD_MS);
    }
    Serial.println("Connected!");

  }
}

}
