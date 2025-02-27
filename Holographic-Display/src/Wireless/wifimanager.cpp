/*
 * @file wifimanager.cpp
 * @authors mia
 * @brief Defines a WifiManager class used for interacting with the WiFi component on the board.
 * @version 0.1.0
 * @date 2024-07.01
 *
 * Copyright Deimo Elektronik GmbH (c) 2024
*/

#include "Wireless/wifimanager.hpp"

namespace Wireless
{

void WifiManager::_begin_AP()
{
  ESP_LOGI(TAG, "Setting up AP with SSID %s...", AP_SSID);

  while (!WiFi.softAP(AP_SSID, AP_PASSWORD))
  {
    ESP_LOGE(TAG, "Failed to set up AP! retrying...");
    delay(1000);
  }
  
  _IP = WiFi.softAPIP();

  ESP_LOGI(TAG, "AP IP: %s", _IP.toString().c_str());
}

void WifiManager::_connect_AP()
{
  ESP_LOGI(TAG, "Connecting to %s...", WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) 
      delay(500);

  _IP = WiFi.localIP();
  
  ESP_LOGI(TAG, "Local IP: %s", _IP.toString().c_str());
}

void WifiManager::begin()
{
  // We have to get the underlying esp-idf WiFi configuration and manually change the WiFis assigned core to 1.
  
#ifndef AP_MODE
  _connect_AP();
#else
  _begin_AP();
#endif
}

}