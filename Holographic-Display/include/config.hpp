/*
 * @file config.hpp
 * @authors Mia
 * @brief Contains various configurations and definitions for the program.
 * @version 0.1.0
 * @date 2024-01-07
 *
 * Copyright Deimo Elektronik GmbH (c) 2024
*/

#pragma once

#include "credentials.hpp"

// The amount of angles the image will be cut into.
#define ANGLES_PER_ROTATION 360

// The amount of LEDs on each strip.
#define LEDS_PER_STRIP 11

// The data pin the LEDs are connected to
#define LED_DATA_PIN 2
// #define LED_CLOCK_PIN 11

// Defines the width/height of the image to create.
// This is equal to the number of LED's per strip times 2.
#define IMAGE_SIZE LEDS_PER_STRIP * 2

// Defines the approx. max number of frames that can be loaded.
#define MAX_FRAMES (int)(8000000 / (LEDS_PER_STRIP * 2 * 2 * sizeof(CRGB))) - 10

#define IMAGE_DATA_SIZE (MAX_FRAMES * IMAGE_SIZE * IMAGE_SIZE * sizeof(CRGB))
// Defines the most current image that has been uploaded from the website.
#define IMAGE_JSON_NAME "/datadump/image.json"

// Define this for Over-The-Air sketch/firmware updates.
// - - - - - - WARNING - - - - - - 
// If you disable this, then all OTA Updates will be non-functional.
// Manual reupload with this variable defined is needed, to enable OTA again!
#define OTA_FIRMWARE

// Define to enable mDNS with the specified hostname. 
#define MDNS_HOSTNAME "holo"

// Define the Baudrate the Serial interface will use.
#define SERIAL_BAUDRATE 115200

// Define the port the webserver will use.
// Only 80 is currently supported! 
#define WEBSERVER_PORT 80

// Define to turn the μController into a WiFi Access Point. 
// #define AP_MODE

#ifndef AP_MODE // Connect to a WiFi Access point.
    #ifndef WIFI_PASSWORD
        #error "The Variable "WIFI_PASSWORD" is not set! You can do this by creating a credentials.hpp file in the include directory and defining the variable there!"
    #endif

    #ifndef WIFI_SSID
        #error "The Variable "WIFI_SSID" is not set! You can do this by creating a credentials.hpp file in the include directory and defining the variable there!"
    #endif
#else // Create your own WiFi Access point.
    #ifndef AP_PASSWORD
        #error "The Variable "AP_PASSWORD" is not set! You can do this by creating a credentials.hpp file in the include directory and defining the variable there!"
    #endif

    #ifndef AP_SSID
        #error "The Variable "AP_SSID" is not set! You can do this by creating a credentials.hpp file in the include directory and defining the variable there!"
    #endif
#endif
