
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
#define SERVER_GET_SUFFIX "/TargetPower"

#define MOTOR_PIN 27
#define MOTOR_PWM_FREQUENCY 50
#define MOTOR_MIN 1000
#define MOTOR_MAX 5000

#define HAL_SENSOR1_PIN 17
#define HAL_SENSOR2_PIN 18

#define GET_RPM_DELAY 480
#define SEND_RPM_DELAY 250
#define DEFAULT_DELAY 500

// Define the baudrate the serial interface will use.
#define SERIAL_BAUDRATE 115200