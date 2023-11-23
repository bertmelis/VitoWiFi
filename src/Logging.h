/*
Copyright (c) 2023 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#pragma once

#if defined(ARDUINO_ARCH_ESP32)
  #include <esp32-hal-log.h>
  #include "freertos/FreeRTOS.h"
  #include "freertos/task.h"
  #if defined(DEBUG_VITOWIFI)
  // Logging is en/disabled by Arduino framework macros
    #define vw_log_i(...) log_i(__VA_ARGS__)
    #define vw_log_e(...) log_e(__VA_ARGS__)
    #define vw_log_w(...) log_w(__VA_ARGS__)
  #else
  // Logging is disabled
    #define vw_log_i(...)
    #define vw_log_e(...)
    #define vw_log_w(...)
  #endif
#elif defined(ARDUINO_ARCH_ESP8266)
  #if defined(DEBUG_ESP_PORT) && defined(DEBUG_VITOWIFI)
    #include <Arduino.h>
    #define vw_log_i(...) DEBUG_ESP_PORT.printf(__VA_ARGS__); DEBUG_ESP_PORT.print("\n")
    #define vw_log_e(...) DEBUG_ESP_PORT.printf(__VA_ARGS__); DEBUG_ESP_PORT.print("\n")
    #define vw_log_w(...) DEBUG_ESP_PORT.printf(__VA_ARGS__); DEBUG_ESP_PORT.print("\n")
  #else
    #define vw_log_i(...)
    #define vw_log_e(...)
    #define vw_log_w(...)
  #endif
#else
  // when building for PC, always show debug statements as part of testing suite
  #include <iostream>
  #define vw_log_i(...) std::cout << "[I] " << __FILE__ ":" << __LINE__ << ": "; printf(__VA_ARGS__); std::cout << std::endl
  #define vw_log_e(...) std::cout << "[E] " << __FILE__ ":" << __LINE__ << ": "; printf(__VA_ARGS__); std::cout << std::endl
  #define vw_log_w(...) std::cout << "[W] " << __FILE__ ":" << __LINE__ << ": "; printf(__VA_ARGS__); std::cout << std::endl
#endif
