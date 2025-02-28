
#pragma once

#include <Arduino.h>


struct RGB
{
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    
    RGB(uint8_t red, uint8_t green, uint8_t blue) : r(red), g(green), b(blue) {}
    RGB() {}

    static const RGB Red;
    static const RGB Green;
    static const RGB Blue;
    static const RGB White;
    static const RGB Black;
};
