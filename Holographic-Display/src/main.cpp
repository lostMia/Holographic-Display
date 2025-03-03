
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

  // Disable Watchdog on Core 1, because.. fuck it, that's why.
  disableCore0WDT();

  // Disable Task watchdog.
  esp_task_wdt_delete(xTaskGetCurrentTaskHandle());
  esp_task_wdt_deinit();

  psramInit();

  // Start the wifi manager
  wifimanager.begin();

  // Start the webserver manager + rendering engine.
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
