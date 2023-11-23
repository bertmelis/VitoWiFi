/*
Copyright (c) 2023 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#pragma once

#include <cstdint>
#include <cstddef>

#include "../Constants.h"
#include "../Helpers.h"
#include "../Logging.h"
#include "PacketVS2.h"

namespace VitoWiFiInternals {

class ParserVS2 {
 public:
  ParserVS2();
  ParserResult parse(const uint8_t b);
  const VitoWiFi::PacketVS2& packet() const;
  void reset();

 private:
  VitoWiFi::PacketVS2 _packet;
  enum class ParserStep {
    STARTBYTE,
    PACKETLENGTH,
    PACKETTYPE,
    FLAGS,
    ADDRESS1,
    ADDRESS2,
    PAYLOADLENGTH,
    PAYLOAD,
    CHECKSUM
  }_step;
  uint8_t _payloadLength;
};

}  // end namespace VitoWiFiInternals
