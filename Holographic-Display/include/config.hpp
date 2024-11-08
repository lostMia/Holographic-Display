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
//#define AP_MODE

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
