/*
 * @file rendering.cpp
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
  memset(&_imageData, 0, _imageDataSize);
}

void Renderer::_next_pixel(uint8_t *px, uint8_t *py)
{
  int x = *px;
  int y = *py;
  // Move onto the next Pixel on the X-Axis.
  x++;
  if (x != IMAGE_SIZE)
  {
    *px = x;
    return;
  };

  // Move onto the next Pixel on the Y-Axis and reset X-Axis, if already at the end on the X-Axis.
  x = 0;
  
  y = (y == IMAGE_SIZE) ? 0 : y + 1;
  
  *px = x;
  *py = y;
}

void Renderer::_print_image_data()
 {
  for (uint8_t frameIndex = 0; frameIndex < MAX_FRAMES; frameIndex++)
  {
    for (uint8_t x = 0; x < IMAGE_SIZE; x++)
    {
      for (uint8_t y = 0; y < IMAGE_SIZE; y++)
      {
        Serial.print(_imageData[frameIndex][x][y].Red == 255 ? "#" : " ");
      }
      
      Serial.println();
    }
  }
  
  // for (uint8_t x = 0; x < IMAGE_SIZE; x++)
  // {
  //   for (uint8_t y = 0; y < IMAGE_SIZE; y++)
  //   {
  //     Serial.print(_imageData[0][x][y].Red);
  //     Serial.print(_imageData[0][x][y].Green);
  //     Serial.print(_imageData[0][x][y].Blue);
  //   }
  //   
  //   Serial.println();
  // }
}

// Loads the .json file from the file system into the imageData Array, so it can be used for displaying.
void Renderer::load_image_data()
{
  File file = SPIFFS.open(IMAGE_JSON_NAME, "r", false);

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

  JsonDocument jsonDoc;

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
  uint8_t frameCount = 0;

  for (JsonObject frame : frames) 
  {
    uint16_t delay = frame["delay"];
    JsonArray data = frame["data"];
        
    _delayData[frameCount] = delay;
    
    uint8_t indexCount = 0;
    uint8_t x = 0;
    uint8_t y = 0;
    
    for (int value : data) 
    {
      switch (indexCount) 
      {
        case 0:
            _imageData[frameCount][y][x].Red = value; 
            break;
        case 1:
            _imageData[frameCount][y][x].Green = value; 
            break;
        case 2:
            _imageData[frameCount][y][x].Blue = value; 
            _next_pixel(&x, &y);
          
            break;
        default:
          Serial.println("We are inside of default, this can never happen!!!!");
          break;
      }

      indexCount == 2 ? indexCount = 0 : indexCount++;
    }
    frameCount++;
  }
  
  _print_image_data();
}

}