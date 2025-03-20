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

void Renderer::_print_image_data(uint8_t frame)
{
  ESP_LOGI(TAG, "\n\nFrame: %d\nDelay: %d ms\n", frame, _delay_data[frame]);

  char buffer[IMAGE_LENGTH_PIXELS + 1];
  buffer[IMAGE_LENGTH_PIXELS] = '\0';

  for (uint8_t x = 0; x < IMAGE_LENGTH_PIXELS; x++)
  {
    for (uint8_t y = 0; y < IMAGE_LENGTH_PIXELS; y++)
    {
      uint32_t index = frame * IMAGE_LENGTH_PIXELS * IMAGE_LENGTH_PIXELS + y * IMAGE_LENGTH_PIXELS + x;
      
      buffer[y] = _image_data[index].r == 255 ? '#' : '.';
    }
    
    ESP_LOGI(TAG, "%s", buffer);
  }

  ESP_LOGI(TAG, "\n\n- - - - - - - - - - - - - - - - - - - - \n");
}

void Renderer::_show()
{
    _current_transaction.length = ((LEDS_PER_SIDE * 2 * 4) + 8) * 8;
    _current_transaction.user = NULL;
    _current_transaction.tx_buffer = _led_buffer;
   
    // unsigned long first, second;

    // first = micros();

    // Blocking transfer. 
    // spi_device_polling_transmit(_spi, &_current_transaction);

    // Non blocking transfer.
    spi_device_queue_trans(_spi, &_current_transaction, portMAX_DELAY);

    // second = micros();

    // ESP_LOGI(TAG, "Transaction: %d\n\n", second - first);
}

void Renderer::_change_led(uint8_t index, RGB color)
{
  uint16_t offset = 4 + (index * 4);

  _led_buffer[offset] = 0xE0 | _current_brightness;
  _led_buffer[offset + 1] = color.b;
  _led_buffer[offset + 2] = color.g;
  _led_buffer[offset + 3] = color.r;
}

void Renderer::_copy_to_frame_buffer(uint8_t frame, uint8_t* data)
{
  // Extract the delay data from the frame.
  uint16_t delay;
  memcpy(&delay, data, 2);
  _delay_data[frame] = delay;

  // Copy the frame data into the PSRAM image buffer at the given frame index.
  memcpy(((char*)_image_data) + frame * IMAGE_SIZE_BYTES,
    data + 2,
    IMAGE_SIZE_BYTES
  );

  _max_frame = frame;
  
  // Always reset the current frame counter, incase we have a still image now.
  if (_max_frame == 0)
    _current_frame = 0;
 }

// Loads the .bin file from the file system into the _image_data Array,
// so it can be used for displaying.
void Renderer::_load_image_from_flash()
{
  File file = LittleFS.open(IMAGE_DATA_NAME, "r", false);

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

  // Reset all the delay data.
  memset(_delay_data, 0, MAX_FRAMES * sizeof(uint16_t));
  
  uint16_t frame_index, delay = 0;
  char* image_data_pointer = (char*)_image_data;

  while (file.available()) 
  {
    if (frame_index >= MAX_FRAMES) 
    {
      ESP_LOGE(TAG, "Too many frames, buffer overflow!!!");
      break;
    }

    // Read delay and save it inside of the delay buffer.
    file.readBytes((char*)&delay, 2);
    _delay_data[frame_index] = delay;

    // Read the frame data and write it to the current PSRAM buffer pointer. 
    file.readBytes(
      image_data_pointer,
      IMAGE_SIZE_BYTES
    );

    image_data_pointer += IMAGE_SIZE_BYTES;
    frame_index++;
  }

  file.close();

  _max_frame = frame_index - 1;
  ESP_LOGI(TAG, "Frames loaded: %d", _max_frame);
  
  // _print_image_data(0);
  // for (int i = 0; i < _max_frame + 1; i++)
  //   _print_image_data(i);
}

void Renderer::_update_frame_count()
{
  // If there aren't multiple frames that we need to cycle through.
  if (_max_frame < 2)
    return;
    
  unsigned long now = micros();
  uint32_t delay_us = _delay_data[_current_frame] * 1000;

  // If it's time to switch to the next frame.
  if (now - _last_frame_switch > delay_us)
  {
    // Switch to the next frame.
    _current_frame = _current_frame == _max_frame ?
      0 : _current_frame + 1;
    
    _last_frame_switch = now;
  }
}

void Renderer::_update_degree_count() { _current_degrees = _current_degrees == 359 ? 0 : _current_degrees + 1 ; }

void Renderer::_update_led_colors()
{
  uint16_t index;
  RGB color;
  
  uint16_t offset_degrees = (_current_degrees + options.offset) % 360;

  // Go through all the LEDs and change their current color value.  
  for (uint8_t led_index = 0; led_index < LEDS_PER_SIDE; led_index++)
  {
    // Get the cartesian coordinates the LED should be showing inside of the image at that time.
    auto coordinates = conversion_matrix[offset_degrees][LEDS_PER_SIDE - led_index - 1];
    
    index = _current_frame * IMAGE_LENGTH_PIXELS * IMAGE_LENGTH_PIXELS + coordinates.y * IMAGE_LENGTH_PIXELS + (IMAGE_LENGTH_PIXELS - coordinates.x);

    // Get the color value from the image at those coordinates.
    color = _image_data[index];

    color.r = _add_colors(color.r, options.red_color_adjust);
    color.g = _add_colors(color.g, options.green_color_adjust);
    color.b = _add_colors(color.b, options.blue_color_adjust);

    _change_led(led_index, color);
  }
  
  uint16_t opposite_degrees = (offset_degrees + 180) % 360;
  
  // Go through all the LEDs and change their current color value.  
  for (uint8_t led_index = LEDS_PER_SIDE; led_index < LEDS_PER_SIDE * 2; led_index++)
  {
    // Get the cartesian coordinates the LED should be showing inside of the image at that time.
    auto coordinates = conversion_matrix[opposite_degrees][led_index - LEDS_PER_SIDE];

    index = _current_frame * IMAGE_LENGTH_PIXELS * IMAGE_LENGTH_PIXELS + coordinates.y * IMAGE_LENGTH_PIXELS + (IMAGE_LENGTH_PIXELS - coordinates.x);

    // Get the color value from the image at those coordinates.
    color = _image_data[index];

    color.r = _add_colors(color.r, options.red_color_adjust);
    color.g = _add_colors(color.g, options.green_color_adjust);
    color.b = _add_colors(color.b, options.blue_color_adjust);

    _change_led(led_index, color);
  }
}

void IRAM_ATTR _update_timer_ISR()
{
  // Update the timer delay once every full rotation.
  timerAlarmWrite(g_renderer->_render_loop_timer, g_renderer->options._delay_between_degrees_us, true);

  BaseType_t hptw;
  vTaskNotifyGiveFromISR(g_renderer->_display_loop_task, &hptw);
  
  portYIELD_FROM_ISR(hptw);
}

void IRAM_ATTR _update_rotation_ISR(void* parameter)
{
  Renderer *renderer = (Renderer*)parameter;
  
  renderer->_current_degrees = 180;
}

uint8_t Renderer::_add_colors(uint8_t color, int16_t addition)
{
  int16_t temp_color = (int16_t)color;
  int16_t clamped_color = clamp((temp_color + addition), 0, 255);

  return (uint8_t)clamped_color;
}

void Renderer::begin()
{
  g_renderer = this;
  
  BaseType_t result;

  // Allocate all the image data in PSRAM.
  _image_data = (RGB*)ps_malloc(IMAGE_DATA_SIZE);
  
  for (uint32_t index = 0; index < (IMAGE_DATA_SIZE / sizeof(RGB)); index++)
    _image_data[index] = RGB::Black;
  
  // Initialize the SPI bus.
  spi_bus_initialize(SPI_HOST, &_buscfg, SPI_DMA_CH_AUTO);
  
  // Attach the SPI device.
  spi_bus_add_device(SPI_HOST, &_devcfg, &_spi);

  // Allocate DMA buffer.
  _led_buffer = (uint8_t*)heap_caps_malloc(
    (LEDS_PER_SIDE * 2 * 4) + 8,
     MALLOC_CAP_DMA
  );

  // Write start and end sections.
  memset(_led_buffer, 0x00, 4);
  memset(_led_buffer + 4 + (LEDS_PER_SIDE * 2 * 4), 0xFF, 4);

  for (uint8_t led_index = 0; led_index < LEDS_PER_SIDE * 2; led_index++)
    _change_led(led_index, RGB::Black);

  _show();

  _load_image_from_flash();
  // _print_image_data();

  _render_loop_timer = timerBegin(
    0,
    80,
    true
  );

  result = xTaskCreate(
    _display_loop,
    PSTR("Display Loop"),
    4096,
    this,
    configMAX_PRIORITIES,
    &_display_loop_task
  );

  if (result != pdPASS)
    ESP_LOGE(TAG, "Couldn't allocate enough memory!");

  timerAttachInterrupt(_render_loop_timer, _update_timer_ISR, false);
  timerAlarmWrite(_render_loop_timer, options._delay_between_degrees_us, true);
  timerAlarmEnable(_render_loop_timer);
  
  // attachInterruptArg(digitalPinToInterrupt(HAL_PIN), _update_rotation_ISR, this, FALLING);
}

void Renderer::_display_loop(void *parameter)
{
  Renderer *renderer = (Renderer*)parameter;
  
  while (true)
  {
    ulTaskNotifyTake(true, portMAX_DELAY);

    renderer->_update_degree_count();
    renderer->_update_frame_count();
    renderer->_update_led_colors();
    renderer->_show();
  }
}

void Renderer::set_brightness(uint8_t brightness) 
{ 
  // Only change the current brightness if the leds aren't disabled.
  if (_current_brightness != 0)
    _current_brightness = brightness;

  _saved_brightness = brightness;
}

void Renderer::set_renderer_state(bool enabled)
{
  // Set the brightness to 0 incase the renderer should be disabled.
  // This might seem like a stupid idea but it's far more reliable than disabling the 
  // Task or doing something over the top... trust me.
  enabled ? _current_brightness = _saved_brightness : _current_brightness = 0;
}

void Renderer::refresh_image() { _load_image_from_flash(); }

void Renderer::update_frame(uint8_t frame, uint8_t* data) { _copy_to_frame_buffer(frame, data); }

}

