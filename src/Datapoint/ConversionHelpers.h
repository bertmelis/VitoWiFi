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
#include <cstring>  // strlen
#include <cstdio>  // snprintf

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
  - time not specified in pairs
  - (hours not ordered earliest to latest)
  - other formatting than colons or spaces
  - minutes not rounded to multiples of 10
  - whitespace not trimmed
*/
std::size_t encodeSchedule(const char* schedule, std::size_t len, uint8_t* output);
std::size_t encodeSchedule(const char* schedule, uint8_t* output);

/*
Decodes a byte series to a human-readable schedule consisting of time pairs.
Returns the number of characters written including 0-terminator
(will always result in 48 or 0 in case of an error)

Although passed as variables, the function fails when
- len != 8
- maxLen < 48
*/
std::size_t decodeSchedule(const uint8_t* data, std::size_t len, char* output, std::size_t maxLen);

};  // end namespace VitoWiFi
