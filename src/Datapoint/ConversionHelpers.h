/*
Copyright (c) 2025 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#pragma once

#include <cassert>
#include <cstdint>
#include <cstddef>
#include <cstring>

namespace VitoWiFi {

/*
Encodes a c-string containing a schedule to an 8-byte schedule sequence.
Returns the number of pairs encoded
Per byte: 
  - b7-b3 hour
  - b2-b1 minute * 10

Output needs to be at least 8 bytes

A scedule consists of time pairs with minutes rounded to a multiple of 10min.
Hours can be with leading zero or not.

Valid:
  - 7:30 8:30 15:00 23:50
  - 07:30 8:30 15:00 23:50

Invalid:
  - hours not specified in pairs
  - (hours not ordered earliest to latest)
  - no colons or spaces used
  - minutes not rounded to multiples of 10
*/
std::size_t encodeSchedule(const char* schedule, std::size_t len, uint8_t* output);
std::size_t encodeSchedule(const char* schedule, uint8_t* output);

std::size_t decodeSchedule(const uint8_t* data, std::size_t len, char* output, std::size_t maxLen);

};  // end namespace VitoWiFi
