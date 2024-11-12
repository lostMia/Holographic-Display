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
        // Serial.print(_imageData[frameIndex][x][y].Red == 255 ? "#" : " ");
        Serial.print(_imageData[frameIndex][x][y].r);
        Serial.print(_imageData[frameIndex][x][y].g);
        Serial.print(_imageData[frameIndex][x][y].b);
      }
      
      Serial.println();
    }
  }
}

void Renderer::_draw_led_strip_colors(uint16_t current_degrees)
{
  // Go through all the LEDs and change their current color value.  
  for (uint8_t led_index = 0; led_index < LEDS_PER_STRIP; led_index++)
  {
    // Get the cartesian coordinates the LED should be showing inside of the image at that time.
    auto coordinates = conversion_matrix[current_degrees][LEDS_PER_STRIP - led_index];

    // Get the color value from the image at those coordinates.
    CRGB color = _imageData[current_frame][coordinates.x][coordinates.y];
    
    color.r += options.red_color_adjust;
    color.g += options.green_color_adjust;
    color.b += options.blue_color_adjust;

    _leds[led_index] = color;
  }
  
  uint16_t opposite_degrees = (current_degrees + 180) % 360;
  

  // Note: this is the one that is broken. below

  // Go through all the LEDs and change their current color value.  
  for (uint8_t led_index = LEDS_PER_STRIP; led_index < LEDS_PER_STRIP * 2; led_index++)
  {
    // Get the cartesian coordinates the LED should be showing inside of the image at that time.
    //
    auto coordinates = conversion_matrix[opposite_degrees][led_index - LEDS_PER_STRIP];

    // Get the color value from the image at those coordinates.
    CRGB color = _imageData[current_frame][coordinates.x][coordinates.y];
       
    color.r += options.red_color_adjust;
    color.g += options.green_color_adjust;
    color.b += options.blue_color_adjust;

    _leds[led_index] = color;
  }
 
  FastLED.show();
}

void Renderer::_display_loop(void *parameter)
{
  Renderer *renderer = (Renderer*)parameter;

  uint16_t current_degrees = 0;
  unsigned long current_milliseconds, previous_milliseconds;

  while (true)
  {
    current_milliseconds = millis();

    vTaskDelay(5 / portTICK_PERIOD_MS);

    if (current_milliseconds - previous_milliseconds >= *renderer->delay_between_frames_ms) 
    {
      current_degrees = (current_degrees == 359 ? 0 : current_degrees + 1);

      previous_milliseconds = current_milliseconds;
      
      renderer->_draw_led_strip_colors(current_degrees);
    }
  }
}

void Renderer::init(unsigned long *pdelay_between_frames_ms)
{
  BaseType_t result;

  delay_between_frames_ms = pdelay_between_frames_ms;
  FastLED.addLeds<NEOPIXEL, LED_DATA_PIN>(_leds, LEDS_PER_STRIP * 2);
  FastLED.setBrightness(50);

  result = xTaskCreatePinnedToCore(
    _display_loop,
    PSTR("Display Loop"),
    100000,
    this,
    5,
    &_display_loop_task,
    0
  );
  
  if (result != pdPASS)
    Serial.println(F("Couldn't allocate enough memory!!"));
  
  load_image_data();
}

void Renderer::start()
{
  if (eTaskGetState(_display_loop_task) == eRunning)
    return;

  vTaskResume(_display_loop_task);
}

void Renderer::stop()
{
  if (eTaskGetState(_display_loop_task) == eSuspended)
    return;

  vTaskSuspend(_display_loop_task);
}

// Loads the .json file from the file system into the imageData Array, so it can be used for displaying.
void Renderer::load_image_data()
{
  Serial.println("Loading image from flash....");

  File file = SPIFFS.open(IMAGE_JSON_NAME, "r", false);

  if (!file) 
  {
    Serial.println(F("Failed to open file for reading"));
    return;
  }

  size_t size = file.size();
  if (size == 0) 
  {
    Serial.println(F("File is empty"));
    file.close();
    return;
  }

  JsonDocument jsonDoc;

  // Parse the file contents to the JSON document
  DeserializationError error = deserializeJson(jsonDoc, file);
  if (error) 
  {
    Serial.print(F("Failed to parse JSON: "));
    Serial.println(error.c_str());
    file.close();
    return;
  }

  file.close();

  // If the JSON file has nested arrays or objects
  JsonArray frames = jsonDoc["frames"];
  uint8_t frameCount = 0;
  
  stop();

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
            _imageData[frameCount][y][x].r = value; 
            break;
        case 1:
            _imageData[frameCount][y][x].g = value; 
            break;
        case 2:
            _imageData[frameCount][y][x].b = value; 
            _next_pixel(&x, &y);
            break;
        default:
          break;
      }

      indexCount == 2 ? indexCount = 0 : indexCount++;
    }
    frameCount++;
  }
  
  // _print_image_data();

  start();
}

}

