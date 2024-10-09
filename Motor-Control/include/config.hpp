
/*
 * @file config.hpp
 * @authors Mia
 * @brief Contains various configurations and definitions for the program.
 * @version 0.1.0
 * @date 2024-03-10
 *
 * Copyright Deimo Elektronik GmbH (c) 2024
*/

#pragma once

#define SERVER_DNS_NAME "http://holo.local"
#define SERVER_POST_SUFFIX "/post"

#define MOTOR_PIN 27
#define GET_RPM_DELAY 100
#define SEND_RPM_DELAY 1000

// Define the Baudrate the Serial interface will use.
#define SERIAL_BAUDRATE 115200