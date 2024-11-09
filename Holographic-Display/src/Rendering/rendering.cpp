/*
 * @file webserver.cpp
 * @authors mia
 * @brief Manages the rendering of the images on the display.
 * @version 0.1.0
 * @date 2024-11-09
 *
 * Copyright Deimo Elektronik GmbH (c) 2024
*/

#include "Rendering/rendering.hpp"

namespace Rendering 
{


// Clears the imageData Array.
void Renderer::_clear_image_data()
{
  Serial.println("Clear Image Data");
  memset(&imageData, 0, imageDataSize);
}


 

// Loads the .json file from the file system into the imageData Array, so it can be used for displaying.
void Renderer::load_image_data()
{
  File file = SPIFFS.open("/datadump/image.json", "r", false);

  if (!file) 
  {
    Serial.println("Failed to open file for reading");
    return;
  }

  size_t size = file.size();
  if (size == 0) 
  {
    Serial.println("File is empty");
    file.close();
    return;
  }

  // Create a JSON document with a suitable capacity
  StaticJsonDocument<1024> jsonDoc;

  // Parse the file contents to the JSON document
  DeserializationError error = deserializeJson(jsonDoc, file);
  if (error) 
  {
    Serial.print("Failed to parse JSON: ");
    Serial.println(error.c_str());
    file.close();
    return;
  }

  file.close();

  // If the JSON file has nested arrays or objects
  JsonArray frames = jsonDoc["frames"];
  for (JsonObject frame : frames) 
  {
    int delay = frame["delay"];
    JsonArray data = frame["data"];

    Serial.print("Frame delay: ");
    Serial.println(delay);
    Serial.print("Data: ");
    for (int value : data) 
    {
      Serial.print(value);
      Serial.print(", ");
    }
    Serial.println();
  }
}

}