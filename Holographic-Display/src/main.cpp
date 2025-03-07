
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


void setup() 
{
  Serial.begin(SERIAL_BAUDRATE);

  psramInit();
  
  while (!LittleFS.begin(true, "/littlefs", 10, "littlefs")) 
  {
    Serial.println(F("An Error has occurred while mounting LittleFS"));
    vTaskDelay(pdMS_TO_TICKS(200));
  }
  
  renderer.begin();
  wifimanager.begin();
  server.begin();

#ifndef OTA_FIRMWARE
   // Delete the loop task from the scheduler, as we don't need it.
  vTaskDelete(NULL);
#endif
}

void loop()
{
#ifdef OTA_FIRMWARE
  ElegantOTA.loop();
  vTaskDelay(100);
#endif
}
