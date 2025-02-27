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

Renderer *g_renderer;

// Clears the imageData Array.
void Renderer::_clear_image_data()
{
  ESP_LOGW(TAG, "Clearing image data...");
  memset(&_image_data, 0, IMAGE_DATA_SIZE);
}

void Renderer::_next_pixel(uint8_t *px, uint8_t *py)
{
  uint8_t x = *px;
  uint8_t y = *py;

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
  char buffer[IMAGE_SIZE + 1];
  buffer[IMAGE_SIZE] = '\0';

  for (uint8_t frame_index = 0; frame_index <= _max_frame; frame_index++)
  {
    for (uint8_t x = 0; x < IMAGE_SIZE; x++)
    {
      for (uint8_t y = 0; y < IMAGE_SIZE; y++)
      {
        uint32_t index = frame_index * IMAGE_SIZE * IMAGE_SIZE + y * IMAGE_SIZE + x;
        
        buffer[y] = _image_data[index].r == 255 ? '#' : '.';
      }
      
      ESP_LOGI(TAG, "%s", buffer);
    }
  }
}

// Loads the .json file from the file system into the imageData Array, so it can be used for displaying.
void Renderer::_load_image_from_flash()
{
  ESP_LOGW(TAG, "loading image from flash");

  File file = SPIFFS.open(IMAGE_JSON_NAME, "r", false);

  if (!file) 
  {
    ESP_LOGE(TAG, "Failed to open file for reading");
    return;
  }

  size_t size = file.size();
  if (size == 0) 
  {
    ESP_LOGE(TAG, "File is empty");
    file.close();
    return;
  }

  JsonDocument json_doc;

  // Parse the file contents to the JSON document
  DeserializationError error = deserializeJson(json_doc, file);
  if (error) 
  {
    ESP_LOGE(TAG, "Failed to parse JSON: \n%s", error.c_str());
    file.close();
    return;
  }

  file.close();

  JsonArray frames = json_doc["frames"];
  uint8_t frame_count = 0;
  
  for (JsonObject frame : frames) 
  {
    uint16_t delay = frame["delay"];
    JsonArray data = frame["data"];
        
    _delay_data[frame_count] = delay;
    
    uint8_t index_count = 0;
    uint8_t x = 0;
    uint8_t y = 0;
    uint32_t index = frame_count * IMAGE_SIZE * IMAGE_SIZE + x * IMAGE_SIZE + y;
    
    for (int value : data) 
    {
      switch (index_count) 
      {
        case 0:
            _image_data[index].r = value; 
            break;
        case 1:
            _image_data[index].g = value; 
            break;
        case 2:
            _image_data[index].b = value; 
            _next_pixel(&x, &y);

            index = frame_count * IMAGE_SIZE * IMAGE_SIZE + x * IMAGE_SIZE + y;
            break;
        default:
          break;
      }

      index_count == 2 ? index_count = 0 : index_count++;
    }
    frame_count++;
  }

  _max_frame = frame_count - 1;
}

void Renderer::_draw_led_strip_colors()
{
  // unsigned long current_us, previous_us;
  //
  // previous_us = micros();
  uint32_t index;
  CRGB color;

  // Go through all the LEDs and change their current color value.  
  for (uint8_t led_index = 0; led_index < LEDS_PER_STRIP; led_index++)
  {
    // // Get the cartesian coordinates the LED should be showing inside of the image at that time.
    auto coordinates = conversion_matrix[_current_degrees][LEDS_PER_STRIP - led_index - 1];

    index = _current_frame * IMAGE_SIZE * IMAGE_SIZE + coordinates.y * IMAGE_SIZE + coordinates.x;

    // Get the color value from the image at those coordinates.
    color = _image_data[index];

    color.r = _add_colors(color.r, options.red_color_adjust);
    color.g = _add_colors(color.g, options.green_color_adjust);
    color.b = _add_colors(color.b, options.blue_color_adjust);

    _leds[led_index] = CRGB::Red; 
  }
  
  uint16_t opposite_degrees = (_current_degrees + 180) % 360;
  
  // Go through all the LEDs and change their current color value.  
  for (uint8_t led_index = LEDS_PER_STRIP; led_index < LEDS_PER_STRIP * 2; led_index++)
  {
    // Get the cartesian coordinates the LED should be showing inside of the image at that time.
    auto coordinates = conversion_matrix[opposite_degrees][led_index - LEDS_PER_STRIP];

    index = _current_frame * IMAGE_SIZE * IMAGE_SIZE + coordinates.y * IMAGE_SIZE + coordinates.x;

    // Get the color value from the image at those coordinates.
    color = _image_data[index];
    
    color.r = _add_colors(color.r, options.red_color_adjust);
    color.g = _add_colors(color.g, options.green_color_adjust);
    color.b = _add_colors(color.b, options.blue_color_adjust);
    
    _leds[led_index] = color; 
  }

  // current_us = micros();
  //
  // ESP_LOGI(TAG, "first: %d", current_us - previous_us);
  //
  // previous_us = micros();

  FastLED.show();

  // current_us = micros();
  //
  // ESP_LOGI(TAG, "second: %d", current_us - previous_us);
  //
  // delay(500);
}

void IRAM_ATTR _update_led()
{
  // Update the timer delay and restart the timer.
  timerAlarmWrite(g_renderer->_render_loop_timer, g_renderer->options._delay_between_degrees_us, true);

  BaseType_t hptw;
  vTaskNotifyGiveFromISR(g_renderer->_display_loop_task, &hptw);
  
  portYIELD_FROM_ISR(hptw);
}

uint8_t Renderer::_add_colors(uint8_t color, int16_t addition)
{
  int16_t temp_color = (int16_t)color;
  int16_t clamped_color = clamp((temp_color + addition), 0, 255);

  return (uint8_t)clamped_color;
}

void Renderer::init()
{
  g_renderer = this;
  
  BaseType_t result;

  // Allocate all the image data in PSRAM.
  _image_data = (CRGB*)ps_malloc(IMAGE_DATA_SIZE);

  for (uint32_t index = 0; index < (IMAGE_DATA_SIZE / sizeof(CRGB)); index++)
    _image_data[index] = CRGB::Black;
 
  ESP_LOGI(TAG, "Adding LEDs..");
  // FastLED.addLeds<SK9822, LED_DATA_PIN, LED_CLOCK_PIN, BGR, DATA_RATE_MHZ(1)>(_leds, LEDS_PER_STRIP * 2);
  FastLED.addLeds<SK9822, LED_DATA_PIN, LED_CLOCK_PIN, BGR>(_leds, LEDS_PER_STRIP * 2);
  FastLED.setBrightness(15);
  FastLED.setMaxRefreshRate(0);
  
  // Set everything to black.
  for (uint8_t led_index = 0; led_index < LEDS_PER_STRIP * 2; led_index++)
    _leds[led_index] = CRGB::Black;
  
  FastLED.show();
  
  // _load_image_from_flash();

  _render_loop_timer = timerBegin(
    0,
    80,
    true
  );

  timerAttachInterrupt(_render_loop_timer, _update_led, false);
  timerAlarmWrite(_render_loop_timer, options._delay_between_degrees_us, false);
  timerAlarmEnable(_render_loop_timer);
  
  result = xTaskCreate(
    _display_loop,
    PSTR("Display Loop"),
    100000,
    this,
    configMAX_PRIORITIES,
    &_display_loop_task
  );

  if (result != pdPASS)
    ESP_LOGE(TAG, "Couldn't allocate enough memory!");
}

void Renderer::_display_loop(void *parameter)
{
  Renderer *renderer = (Renderer*)parameter;
  
  while (true)
  {
    ulTaskNotifyTake(true, portMAX_DELAY);
   
    ESP_LOGI(TAG, "%d", renderer->_current_degrees);

    renderer->_current_degrees = (g_renderer->_current_degrees == 359 ? 0 : g_renderer->_current_degrees + 1);

    taskENTER_CRITICAL(&optionsMUX);
    bool leds_enabled = renderer->options.leds_enabled;
    taskEXIT_CRITICAL(&optionsMUX);
    
    if (leds_enabled)
      renderer->_draw_led_strip_colors();
  };
}

// void Renderer::start_renderer()
// {
//   if (xTimerStart(_render_loop_timer, pdMS_TO_TICKS(1000)) == pdFAIL)
//   {
//     ESP_LOGI(TAG, "Couldn't start timer!");
//     return;
//   }
// }
//
// void Renderer::stop_renderer()
// {
//   if (xTimerStop(_render_loop_timer, pdMS_TO_TICKS(1000)) == pdFAIL)
//   {
//     ESP_LOGI(TAG, "Couldn't stop timer!");
//     return;
//   }
// }

void Renderer::start_renderer()
{
  if (eTaskGetState(_display_loop_task) == eRunning)
    return;
  
  vTaskResume(_display_loop_task);
}

void Renderer::stop_renderer()
{
  if (eTaskGetState(_display_loop_task) == eSuspended)
    return;

  vTaskSuspend(_display_loop_task);
}

void Renderer::refresh_image()
{
  stop_renderer();
  _load_image_from_flash();
  start_renderer();
}

}

