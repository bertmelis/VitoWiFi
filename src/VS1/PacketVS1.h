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
#include <cassert>

#include "../Constants.h"
#include "../Helpers.h"
#include "../Logging.h"

namespace VitoWiFi {

class PacketVS1 {
 public:
  PacketVS1();
  ~PacketVS1();
  PacketVS1 (const PacketVS1&) = delete;
  PacketVS1& operator =(const PacketVS1&) = delete;
  operator bool() const;
  /* uint8_t operator[](std::size_t index) const; */
  uint8_t& operator[](std::size_t index);

 public:
  bool createPacket(uint8_t packetType, uint16_t addr, uint8_t len, const uint8_t* data = nullptr);
  bool setLength(uint8_t length);
  uint8_t length() const;
  uint8_t packetType() const;
  uint16_t address() const;
  uint8_t dataLength() const;
  const uint8_t* data() const;

  void reset();

 protected:
  std::size_t _allocatedLength;
  uint8_t* _buffer;
};

}  // end namespace VitoWiFi
