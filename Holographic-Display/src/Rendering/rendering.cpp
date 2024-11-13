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
    auto coordinates = conversion_matrix[current_degrees][LEDS_PER_STRIP - led_index - 1];

    // Get the color value from the image at those coordinates.
    CRGB color = _imageData[_current_frame][coordinates.x][coordinates.y];
    
    // color.r = _add_colors(color1, color2);
    // color.g = _add_colors(color.g, options.green_color_adjust);
    // color.b = _add_colors(color.b, options.blue_color_adjust);






    _leds[led_index] = color;
  }
  
  uint16_t opposite_degrees = (current_degrees + 180) % 360;
  
  // Go through all the LEDs and change their current color value.  
  for (uint8_t led_index = LEDS_PER_STRIP; led_index < LEDS_PER_STRIP * 2; led_index++)
  {
    // Get the cartesian coordinates the LED should be showing inside of the image at that time.
    auto coordinates = conversion_matrix[opposite_degrees][led_index - LEDS_PER_STRIP];

    // Get the color value from the image at those coordinates.
    CRGB color = _imageData[_current_frame][coordinates.x][coordinates.y];

    // _add_colors(&color.r, &options.red_color_adjust);
    // _add_colors(&color.g, &options.green_color_adjust);
    // _add_colors(&color.b, &options.blue_color_adjust);

    _leds[led_index] = color;
  }
  
  FastLED.show();
}

void Renderer::_display_loop(void *parameter)
{
  Renderer *renderer = (Renderer*)parameter;

  uint16_t current_degrees = 0;
  unsigned long current_microseconds, previous_microseconds;
  
  *renderer->_delay_between_degrees_us = (unsigned long)1;

  while (true)
  {
    current_microseconds = micros();

    if (current_microseconds - previous_microseconds >= *renderer->_delay_between_degrees_us) 
    {
      current_degrees = (current_degrees == 359 ? 0 : current_degrees + 1);

      previous_microseconds = current_microseconds;
      
      if (renderer->options.leds_enabled)
        renderer->_draw_led_strip_colors(current_degrees);
    }
  }
}

uint8_t _add_colors(uint8_t color, int16_t addition)
{
  int16_t temp_color = (int16_t)color;
  return (uint8_t)clamp((int)(temp_color + addition), 0, 255);
}

void Renderer::init(unsigned long *pdelay_between_degrees_us)
{
  BaseType_t result;

  _delay_between_degrees_us = pdelay_between_degrees_us;

  // Disable Watchdog on core 0, as the renderer must not lag behind or have any disturbances and
  // the entire core is getting blocked.
  disableCore0WDT();

  FastLED.addLeds<NEOPIXEL, LED_DATA_PIN>(_leds, LEDS_PER_STRIP * 2);
  // FastLED.addLeds<SK9822, LED_DATA_PIN, LED_CLOCK_PIN, RGB>(_leds, LEDS_PER_STRIP * 2);
  FastLED.setBrightness(20);
  FastLED.setMaxRefreshRate(0);

  result = xTaskCreatePinnedToCore(
    _display_loop,
    PSTR("Display Loop"),
    100000,
    this,
    32,
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

