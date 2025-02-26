
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

// #define SERVER_DNS_NAME "http://192.168.241.38"
#define SERVER_POST_SUFFIX "/post"
#define SERVER_GET_SUFFIX "/TargetPower"


#define MOTOR_PWM_SEND_PIN 11
#define MOTOR_PULSE_FEEDBACK_PIN 12
// Do NOT modify while the motor is running!! Should alway be on high!
#define MOTOR_DIRECTION_WRITE_PIN 10

#define MOTOR_PWM_FREQUENCY 30000 // 20 to 30kHz
#define MOTOR_PWM_CHANNEL 0
#define MOTOR_PWM_RESOLUTION 8

#define GET_RPM_DELAY 480
#define SEND_RPM_DELAY 250
#define DEFAULT_DELAY 500

// Define the baudrate the serial interface will use.
#define SERIAL_BAUDRATE 115200