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
  #define millis() std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count()
#endif

#define vw_abort() abort()
