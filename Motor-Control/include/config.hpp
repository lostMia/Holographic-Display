
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


#define MOTOR_PWM_SEND_PIN 11
#define MOTOR_PULSE_FEEDBACK_PIN 12
// Do NOT modify while the motor is running!! Should alway be on high!
#define MOTOR_DIRECTION_WRITE_PIN 10

#define MOTOR_PWM_FREQUENCY 30000 // 20 to 30kHz
#define MOTOR_PWM_CHANNEL 0
#define MOTOR_PWM_RESOLUTION 8

// The maximum allowed time between each pulse to still be considered rotating
// Comes out to about a fitfh rotation every second.
#define LAST_PULSE_MAX_DELAY_US 200000

// #define USE_AVERAGED_DELAY 
#ifdef USE_AVERAGED_DELAY
#define LAST_PULSES_TO_AVERAGE 5 
#endif

// The timings for when to send the different updates.
#define GET_RPM_DELAY 500
#define SEND_RPM_DELAY 200

#define DEFAULT_DELAY 500

// Define the baudrate the serial interface will use.
#define SERIAL_BAUDRATE 115200