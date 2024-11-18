
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
  
  delay(2000);

  Serial.print("Total heap:  ");
  Serial.println(ESP.getHeapSize());
  Serial.print("Free heap:   ");
  Serial.println(ESP.getFreeHeap());
  Serial.print("Total PSRAM: ");
  Serial.println(ESP.getPsramSize());
  Serial.print("Free PSRAM:  ");
  Serial.println(ESP.getFreePsram());
  Serial.print("Used PSRAM:  ");
  Serial.println(ESP.getPsramSize() - ESP.getFreePsram());
  Serial.println("begin");

  int buffer_size = IMAGE_SIZE * IMAGE_SIZE * sizeof(CRGB);
  unsigned long start, end;

  CRGB color;
  int counter = 0;

  CRGB(*RAM_buffer)[IMAGE_SIZE] = (CRGB(*)[IMAGE_SIZE])malloc(buffer_size);
  CRGB(*PSRAM_buffer)[IMAGE_SIZE] = (CRGB(*)[IMAGE_SIZE])ps_malloc(buffer_size);

  for (int x = 0; x < IMAGE_SIZE; x++) {
    for (int y = 0; y < IMAGE_SIZE; y++) {
        RAM_buffer[x][y] = CRGB::Red;
        PSRAM_buffer[x][y] = CRGB::Red;
    }
}

  Serial.println("Regular RAM:");
  start = micros();
  
  // do things in here.
  for (uint8_t led_index = 0; led_index < 64; led_index++)
  {
    color = RAM_buffer[led_index][led_index];
    
    if (color == CRGB::Red)
      RAM_buffer[led_index][led_index] = CRGB::Black;
    
    counter++;
  }


  Serial.println(counter);
  end = micros();

  Serial.println(end - start);
  

  counter = 0;
  Serial.println("PSRAM:");
  start = micros();

  // do things in here.
  for (uint8_t led_index = 0; led_index < 64; led_index++)
  {
    color = PSRAM_buffer[led_index][led_index];
    
    if (color == CRGB::Red)
      PSRAM_buffer[led_index][led_index] = CRGB::Black;
    
    counter++;
  }
  Serial.println(counter);
  
  end = micros();
  
  Serial.println(end - start);


  Serial.print("Used PSRAM:  ");
  Serial.println(ESP.getPsramSize() - ESP.getFreePsram());
  
  delay(10000000);

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
