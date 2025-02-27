/*
 * @file rendering.hpp
 * @authors mia
 * @brief Manages the rendering of the images on the display.
 * @version 0.1.0
 * @date 2024-11-09
 *
 * Copyright Deimo Elektronik GmbH (c) 2024
*/

#pragma once

#include <Arduino.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <iomanip>
#include <string>
#include <stdio.h>
#include <iostream>
#include <string>
#include <sstream>
#include "config.hpp"
#include "conversion_matrix.hpp"
#include "esp_log.h"

// #define FASTLED_ESP32_SPI_BULK_TRANSFER 1
// #define FASTLED_ALL_PINS_HARDWARE_SPI
// #define FASTLED_ESP32_SPI_BULK_TRANSFER_SIZE 100000

#include "FastLED.h"

using namespace std;


namespace Rendering 
{

static portMUX_TYPE optionsMUX = portMUX_INITIALIZER_UNLOCKED;


struct Options 
{
    int16_t red_color_adjust = 0;
    int16_t green_color_adjust = 0;
    int16_t blue_color_adjust = 0;
    unsigned long _delay_between_degrees_us = 5000;
    bool leds_enabled = true;
};


void IRAM_ATTR _update_led();

// Class managing the displaying of images using the led strips.
class Renderer
{
private:
    // Allocate the image inside of PSRAM
    CRGB* _image_data;
    uint16_t _delay_data[MAX_FRAMES];
    TaskHandle_t _display_loop_task = NULL;
    hw_timer_t* _render_loop_timer;
    uint8_t _current_frame = 0;
    uint8_t _current_degrees = 0;
    uint8_t _max_frame = 0;

    void _clear_image_data();
    void _print_image_data();
    void _load_image_from_flash();
    void _next_pixel(uint8_t *x, uint8_t *y);
    void _draw_led_strip_colors();
    static void _display_loop(void *parameter);
    friend void IRAM_ATTR _update_led();
    uint8_t _add_colors(uint8_t color, int16_t addition);

public:
    CRGB _leds[LEDS_PER_STRIP * 2];
    Options options;
    
    void init();
    void start_renderer();
    void stop_renderer();
    void refresh_image();
};

extern Renderer *g_renderer;

}
