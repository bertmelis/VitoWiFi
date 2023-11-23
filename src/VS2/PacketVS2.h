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
#include <cassert>  // assert

#include "../Constants.h"
#include "../Helpers.h"
#include "../Logging.h"

namespace VitoWiFiInternals {

class ParserVS2;

}  // end namespace VitoWiFiInternals

namespace VitoWiFi {

class PacketVS2 {
  friend class VitoWiFiInternals::ParserVS2;

 public:
  PacketVS2();
  ~PacketVS2();
  PacketVS2 (const PacketVS2&) = delete;
  PacketVS2& operator =(const PacketVS2&) = delete;
  operator bool() const;
  /* uint8_t operator[](std::size_t index) const; */
  uint8_t& operator[](std::size_t index);

 public:
  bool createPacket(PacketType pt, FunctionCode fc, uint8_t id, uint16_t addr, uint8_t len, const uint8_t* data = nullptr);
  bool setLength(uint8_t length);
  uint8_t length() const;
  PacketType packetType() const;
  FunctionCode functionCode() const;
  uint8_t id() const;
  uint16_t address() const;
  uint8_t dataLength() const;
  const uint8_t* data() const;

  uint8_t checksum() const;

  void reset();

 protected:
  std::size_t _allocatedLength;
  uint8_t* _buffer;
};

}  // end namespace VitoWiFi
