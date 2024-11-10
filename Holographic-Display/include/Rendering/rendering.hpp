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
#include <iostream>
#include <string>
#include <sstream>
#include "config.hpp"
#include "conversion_matrix.hpp"
#include "FastLED.h"



namespace Rendering 
{


class Renderer
{
private:
    CRGB _imageData[MAX_FRAMES][IMAGE_SIZE][IMAGE_SIZE];
    uint16_t _delayData[MAX_FRAMES];
    size_t _imageDataSize = MAX_FRAMES * IMAGE_SIZE * IMAGE_SIZE * sizeof(CRGB);
    TaskHandle_t _display_loop_task = NULL;
    unsigned long *delay_between_frames_ms;
    uint8_t current_frame = 0;
    CRGB _leds[LEDS_PER_STRIP];
    

    void _clear_image_data();
    void _print_image_data();
    void _next_pixel(uint8_t *x, uint8_t *y);
    static void _display_loop(void *parameter);
    void _draw_led_strip_colors(uint16_t current_degrees);

public:
    void init(unsigned long *pdelay_between_frames_ms);
    void start();
    void stop();
    void load_image_data();
};

}
