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
  if (x != IMAGE_LENGTH_PIXELS)
  {
    *px = x;
    return;
  };

  // Move onto the next Pixel on the Y-Axis and reset X-Axis, if already at the end on the X-Axis.
  x = 0;
  
  y = (y == IMAGE_LENGTH_PIXELS) ? 0 : y + 1;
  
  *px = x;
  *py = y;
}

void Renderer::_print_image_data()
{
  char buffer[IMAGE_LENGTH_PIXELS + 1];
  buffer[IMAGE_LENGTH_PIXELS] = '\0';

  for (uint16_t frame_index = 0; frame_index <= _max_frame; frame_index++)
  {
    for (uint8_t x = 0; x < IMAGE_LENGTH_PIXELS; x++)
    {
      for (uint8_t y = 0; y < IMAGE_LENGTH_PIXELS; y++)
      {
        uint32_t index = frame_index * IMAGE_LENGTH_PIXELS * IMAGE_LENGTH_PIXELS + y * IMAGE_LENGTH_PIXELS + x;
        
        buffer[y] = _image_data[index].r == 255 ? '#' : '.';
      }
      
      ESP_LOGI(TAG, "%s", buffer);
    }
  }
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

// Loads the .bin file from the file system into the _image_data Array,
// so it can be used for displaying.
void Renderer::_load_image_from_flash()
{
  File file = SPIFFS.open(IMAGE_DATA_NAME, "r", false);

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
  
  const uint16_t chunk_size = 128;
  uint16_t frame_index = 0;
  char chunk_buffer[chunk_size * sizeof(RGB)];

   while (file.available()) 
  {
    if (frame_index >= MAX_FRAMES) 
    {
      ESP_LOGE(TAG, "Too many frames, buffer overflow!!!");
      break;
    }

    // Read delay and save it inside of the delay buffer.
    uint16_t delay;
    file.readBytes((char*)&delay, 2);

    _delay_data[frame_index] = delay;

    for (uint8_t i = 0; i < IMAGE_SIZE_BYTES / (chunk_size * sizeof(RGB)); i++)
    {
      file.readBytes(
        chunk_buffer,
        chunk_size * sizeof(RGB)
      );
      
      // Pointer Magic :trademark:
      memcpy((void*)&_image_data[chunk_size * i],
        chunk_buffer,
        chunk_size * sizeof(RGB)
      );
    }

    frame_index++;
  }

  file.close();

  ESP_LOGI(TAG, "Frame Count: %d", frame_index);
}

void Renderer::_update_led_colors()
{
  uint16_t index;
  RGB color;

  // Go through all the LEDs and change their current color value.  
  for (uint8_t led_index = 0; led_index < LEDS_PER_SIDE; led_index++)
  {
    // Get the cartesian coordinates the LED should be showing inside of the image at that time.
    auto coordinates = conversion_matrix[_current_degrees][LEDS_PER_SIDE - led_index - 1];
    
    index = _current_frame * IMAGE_LENGTH_PIXELS * IMAGE_LENGTH_PIXELS + coordinates.y * IMAGE_LENGTH_PIXELS + coordinates.x;

    // Get the color value from the image at those coordinates.
    color = _image_data[index];

    color.r = _add_colors(color.r, options.red_color_adjust);
    color.g = _add_colors(color.g, options.green_color_adjust);
    color.b = _add_colors(color.b, options.blue_color_adjust);

    _change_led(led_index, color);
  }
  
  uint16_t opposite_degrees = (_current_degrees + 180) % 360;
  
  // Go through all the LEDs and change their current color value.  
  for (uint8_t led_index = LEDS_PER_SIDE; led_index < LEDS_PER_SIDE * 2; led_index++)
  {
    // Get the cartesian coordinates the LED should be showing inside of the image at that time.
    auto coordinates = conversion_matrix[opposite_degrees][led_index - LEDS_PER_SIDE];

    index = _current_frame * IMAGE_LENGTH_PIXELS * IMAGE_LENGTH_PIXELS + coordinates.y * IMAGE_LENGTH_PIXELS + coordinates.x;

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
  // Update the timer delay and restart the timer.
  timerAlarmWrite(g_renderer->_render_loop_timer, g_renderer->options._delay_between_degrees_us, true);

  BaseType_t hptw;
  vTaskNotifyGiveFromISR(g_renderer->_display_loop_task, &hptw);
  
  portYIELD_FROM_ISR(hptw);
}

void IRAM_ATTR _update_rotation_ISR(void* parameter)
{
  ESP_LOGI("you should never appear", "THIS SHOULD NEVER APPEAR");

  Renderer *renderer = (Renderer*)parameter;
  
  renderer->_current_degrees = 0;
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

  result = xTaskCreatePinnedToCore(
    _display_loop,
    PSTR("Display Loop"),
    4096,
    this,
    configMAX_PRIORITIES,
    &_display_loop_task,
    RENDERER_CORE
  );

  if (result != pdPASS)
    ESP_LOGE(TAG, "Couldn't allocate enough memory!");

  timerAttachInterrupt(_render_loop_timer, _update_timer_ISR, false);
  timerAlarmWrite(_render_loop_timer, options._delay_between_degrees_us, false);
  timerAlarmEnable(_render_loop_timer);
  
  // attachInterruptArg(digitalPinToInterrupt(HAL_PIN), _update_rotation_ISR, this, RISING);
}

void Renderer::_display_loop(void *parameter)
{
  Renderer *renderer = (Renderer*)parameter;
  
  while (true)
  {
    ulTaskNotifyTake(true, portMAX_DELAY);
   
    // ESP_LOGI(TAG, "%d", renderer->_current_degrees);

    renderer->_current_degrees = (g_renderer->_current_degrees == 359 ? 0 : g_renderer->_current_degrees + 1);
    

    if (renderer->_current_degrees == 0)
      digitalWrite(6, HIGH);
    else if (renderer->_current_degrees == 180)
      digitalWrite(6, LOW);

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

void Renderer::refresh_image()
{
  _load_image_from_flash();
}

}

