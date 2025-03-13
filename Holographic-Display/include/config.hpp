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

#define TAG ""

// The amount of angles the image will be cut into.
#define ANGLES_PER_ROTATION 360

// The amount of LEDs on each strip.
#define LEDS_PER_SIDE 64

// The data pin the LEDs are connected to
#define LED_DATA_PIN 7
#define LED_CLOCK_PIN 4

// The data pin the HAL sensor is connected to
#define HAL_PIN 10

// Defines the width/height of the image to create.
// This is equal to the number of LED's per strip times 2.
#define IMAGE_LENGTH_PIXELS (LEDS_PER_SIDE * 2)
// The image size in bytes.
#define IMAGE_SIZE_BYTES (IMAGE_LENGTH_PIXELS * IMAGE_LENGTH_PIXELS * sizeof(RGB))
// The image size in pixels.
#define IMAGE_SIZE_PIXELS (IMAGE_LENGTH_PIXELS * IMAGE_LENGTH_PIXELS)

// Defines the max number of frames that can be loaded. 
// The PSRAM size is 8MB! Yes, MB, not MiB. 
// That means we can store up to 8.000.000 Bytes.
// 8.000.000/(128*128*3) = 162.76
// Therefore we can store up to 162 Images in the PSRAM.
#define MAX_FRAMES 162

#define IMAGE_DATA_SIZE (MAX_FRAMES * IMAGE_LENGTH_PIXELS * IMAGE_LENGTH_PIXELS * sizeof(RGB))
// Defines the most current image that has been uploaded from the website.
#define IMAGE_DATA_NAME "/data.bin"

// Define this for Over-The-Air sketch/firmware updates.
// - - - - - - WARNING - - - - - - 
// If you disable this, then all OTA Updates will be non-functional.
// Manual reupload with this variable defined is needed to enable OTA again!
#define OTA_FIRMWARE
// Make sure to add this this in the ElegantOTA.h file!!
// #define ELEGANTOTA_USE_ASYNC_WEBSERVER 1

// Define to enable mDNS with the specified hostname. 
#define MDNS_HOSTNAME "holo"
// #define CONFIG_FREERTOS_UNICORE

// Define the Baudrate the Serial interface will use.
#define SERIAL_BAUDRATE 115200

// Define the port the webserver will use.
// Only 80 is currently supported! 
#define WEBSERVER_PORT 80

// The SPI host to run the LEDs on.
#define SPI_HOST SPI3_HOST
// The frequency of the SPI channel that is used to talk to the 
// LED strip. (in MHz!)
#define SPI_FREQUENCY 45

// Which of the cores on the ESP the specific tasks are supposed to run on.
#define RENDERER_CORE 0
// #define CONFIG_ESP32_WIFI_TASK_PINNED_TO_CORE_1 0
// #define CONFIG_MDNS_TASK_AFFINITY 1

// #define configTICK_RATE_HZ 100
// #define configUSE_PREEMPTION 1

// Define to turn the μController into a WiFi Access Point. 
#define AP_MODE

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
