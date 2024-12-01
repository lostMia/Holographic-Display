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
  Serial.print(F("Setting up AP with SSID "));
  Serial.print(AP_SSID);
  Serial.print(F("..."));

  while (!WiFi.softAP(AP_SSID, AP_PASSWORD))
  {
    Serial.println(F("Failed to set up AP"));
    delay(500);
  }

  Serial.println(F("Done"));
  
  _IP = WiFi.softAPIP();

  Serial.print(F("AP IP:"));
  Serial.println(_IP);
}

void WifiManager::_connect_AP()
{
  Serial.print(F("Connecting to "));
  Serial.println(WIFI_SSID);
  Serial.print(F("..."));

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) 
  {
      delay(500);
      Serial.print(F("."));
  }

  Serial.println(F("Done"));
  
  _IP = WiFi.localIP();
  
  Serial.print(F("Local IP:"));
  Serial.println(_IP);
}

void WifiManager::begin()
{
  // We have to get the underlying esp-idf WiFi configuration and manually change the WiFis assigned core to 1.
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  cfg.wifi_task_core_id = 1;

  esp_wifi_init(&cfg);
  
#ifndef AP_MODE
  _connect_AP();
#else
  _begin_AP();
#endif
}

}