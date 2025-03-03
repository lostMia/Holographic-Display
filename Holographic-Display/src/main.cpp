
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

  // Disable Watchdog on core 0, as the renderer must not lag behind or have any disturbances and
  // the entire core is getting blocked.
  disableCore0WDT();
  disableCore1WDT();

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
#endif
}
