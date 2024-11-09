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
#include "FastLED.h"


namespace Rendering 
{

struct ColorValue {
    uint8_t Red;
    uint8_t Green;
    uint8_t Blue;
};


class Renderer
{
private:
    // [frame][height][width](ColorValue) = [frame Count] = 128 * 128 * 3 = frames * 49152 Bytes
    ColorValue imageData[MAX_FRAMES][IMAGE_SIZE][IMAGE_SIZE];
    size_t imageDataSize = MAX_FRAMES * IMAGE_SIZE * IMAGE_SIZE * sizeof(ColorValue);

    void _clear_image_data();

public:
    void load_image_data();

};

}
