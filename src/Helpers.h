/*
Copyright (c) 2023 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#pragma once

#include <cstdlib>
#include <cstdint>
#include <cstddef>

#if defined(__linux__)
  #include <chrono>  // NOLINT [build/c++11]
  #define vw_millis() std::chrono::duration_cast<std::chrono::duration<uint32_t, std::milli>>(std::chrono::system_clock::now().time_since_epoch()).count()
#elif defined(ESP_PLATFORM)
  #include "freertos/FreeRTOS.h"
  #define vw_millis() xTaskGetTickCount() * portTICK_PERIOD_MS
#elif defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)
  #define vw_millis() millis()
#else
#error "Unsupported target platform"
#endif

#define vw_abort() abort()
