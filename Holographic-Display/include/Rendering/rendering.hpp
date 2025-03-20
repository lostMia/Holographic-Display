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
#include <LittleFS.h>
#include <iomanip>
#include <stdio.h>
#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include "config.hpp"
#include "conversion_matrix.hpp"
#include "rgb.hpp"
#include "esp_log.h"
#include "driver/spi_master.h"


using namespace std;


namespace Rendering 
{

static portMUX_TYPE optionsMUX = portMUX_INITIALIZER_UNLOCKED;

struct Options 
{
    int16_t red_color_adjust = 0;
    int16_t green_color_adjust = 0;
    int16_t blue_color_adjust = 0;
    uint16_t offset = 0;
    unsigned long _delay_between_degrees_us = 5000;
};


void IRAM_ATTR _update_timer_ISR();
void IRAM_ATTR _update_rotation_ISR(void* parameter);

// Class managing the displaying of images using the led strips.
class Renderer
{
private:
    // Allocate the image inside of PSRAM
    RGB* _image_data;
    uint16_t _delay_data[MAX_FRAMES];

    TaskHandle_t _display_loop_task = NULL;
    hw_timer_t* _render_loop_timer;
    
    spi_device_handle_t _spi;

    spi_bus_config_t _buscfg = {
        .mosi_io_num = LED_DATA_PIN,
        .miso_io_num = -1,
        .sclk_io_num = LED_CLOCK_PIN,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = (LEDS_PER_SIDE * 2 * 4) + 8,
    };

    spi_device_interface_config_t _devcfg = {
        .mode = 0,                          // SPI mode 0 (CPOL=0, CPHA=0)
        .clock_speed_hz = SPI_FREQUENCY * 1000 * 1000,
        .spics_io_num = -1,
        .flags = SPI_DEVICE_HALFDUPLEX,
        .queue_size = 1,
    };
    
    spi_transaction_t _current_transaction = {
        .length = ((LEDS_PER_SIDE * 2 * 4) + 8) * 8,  // Bits!
        .user = NULL,
        .tx_buffer = _led_buffer
    };
    

    uint8_t* _led_buffer = NULL;
    uint8_t _current_brightness = 1;
    uint8_t _saved_brightness = 1;
    uint16_t _current_frame= 0;
    uint16_t _current_degrees = 0;
    uint16_t _max_frame = 0;
    unsigned long _last_frame_switch = 0;

    void _clear_image_data();
    void _print_image_data(uint8_t frame);
    void _print_first_pixel();
    void _load_image_from_flash();
    void _copy_to_frame_buffer(uint8_t frame, uint8_t* data);
    void _update_frame_count();
    void _update_degree_count();
    void _update_led_colors();
    void _show();
    void _change_led(uint8_t index, RGB color);
    static void _display_loop(void *parameter);

    // Add the ISR function as friends.
    friend void IRAM_ATTR _update_timer_ISR();
    friend void IRAM_ATTR _update_rotation_ISR(void* parameter);

    uint8_t _add_colors(uint8_t color, int16_t addition);

public:
    Options options;
    
    void begin();
    void set_brightness(uint8_t brightness);
    void set_renderer_state(bool enabled);
    void refresh_image();
    void update_frame(uint8_t frame, uint8_t* data);
};

extern Renderer *g_renderer;

}
