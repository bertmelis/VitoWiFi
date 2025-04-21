/*
Copyright (c) 2023 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#include "Constants.h"

namespace VitoWiFi {

const char* errorToString(OptolinkResult error) {
  if (error == VitoWiFi::OptolinkResult::TIMEOUT) {
    return "timeout";
  } else if (error == VitoWiFi::OptolinkResult::LENGTH) {
    return "length";
  } else if (error == VitoWiFi::OptolinkResult::NACK) {
    return "nack";
  } else if (error == VitoWiFi::OptolinkResult::CRC) {
    return "crc";
  } else if (error == VitoWiFi::OptolinkResult::ERROR) {
    return "error";
  }
  return "invaled error";
}

}  // end namespace VitoWiFi
