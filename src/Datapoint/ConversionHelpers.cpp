/*
Copyright (c) 2023 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#include "ConversionHelpers.h"

bool isDigit(const char c) {
  return c >= '0' && c <= '9';
}

namespace VitoWiFi {

std::size_t encodeSchedule(const char* schedule, std::size_t len, uint8_t* output) {
  enum ScheduleParserStep {
    Hours1,
    Hours2,
    Minutes1,
    Minutes2,
    Space,
  };

  std::size_t i = 0 - 1;  // first operation in iteration is ++i
  std::size_t scheduleIndex = 0;
  ScheduleParserStep step = ScheduleParserStep::Hours1;
  unsigned int hour = 0;
  unsigned int minutes = 0;

  while (++i < len) {
    if (step == ScheduleParserStep::Hours1) {
      if (isDigit(schedule[i])) {
        hour = schedule[i] - '0';
        step = ScheduleParserStep::Hours2;
        continue;
      }
    } else if (step == ScheduleParserStep::Hours2) {
      if (isDigit(schedule[i])) {
        hour = hour * 10 + (schedule[i] - '0');
        continue;
      } else if (schedule[i] == ':') {
        step = ScheduleParserStep::Minutes1;
        continue;
      }
    } else if (step == ScheduleParserStep::Minutes1) {
      if (isDigit(schedule[i])) {
        minutes = schedule[i] - '0';
        step = ScheduleParserStep::Minutes2;
        continue;
      }
    } else if (step == ScheduleParserStep::Minutes2) {
      if (schedule[i] == '0') {
        minutes = minutes * 10 + (schedule[i] - '0');
        // parsing is possibly complete
        if (hour <= 23 || minutes <= 59) {
          output[scheduleIndex] = (0xF8 & hour << 3) | minutes / 10;
          ++scheduleIndex;
          step = ScheduleParserStep::Space;
          continue;
        }
      }
    } else {  // step == ScheduleParserStep::Space
      if (schedule[i] == ' ') {
        step = ScheduleParserStep::Hours1;
        continue;
      }
    }
    return 0;
  }
  if (scheduleIndex % 2 == 0 && step == ScheduleParserStep::Space) {
    // TODO(bertmelis): hours have to be ordered
    return (scheduleIndex + 1) / 2;
  }
  return 0;
}

std::size_t encodeSchedule(const char* schedule, uint8_t* output) {
  return encodeSchedule(schedule, strlen(schedule), output);
}

/*
std::size_t decodeSchedule(const uint8_t* data, std::size_t len, char* output, std::size_t maxLen) {
  assert(len == 8);
  assert(maxLen >= 48);  // 8 times 07:30, 7 spaces and 0-terminator --> 8 * 5 + 7 * 1 + 1

  std::size_t pos = 0;
  for (std::size_t i = 0; i < 8; ++i) {
    unsigned int hour = std::min(23, data[i] >> 3);
    unsigned int minutes = std::min(59, (data[i] & 0x07) * 10);
    int res = snprintf("%u.02:%u.02", hour, minutes);
    if (i < 7) {

    }
  }
}
*/

}  // end namespace VitoWiFi
