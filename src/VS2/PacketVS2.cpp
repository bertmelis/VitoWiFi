/*
Copyright (c) 2023 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#include "PacketVS2.h"

namespace VitoWiFi {

PacketVS2::PacketVS2()
: _allocatedLength(START_PAYLOAD_LENGTH + 6)
, _buffer(nullptr) {
  _buffer = reinterpret_cast<uint8_t*>(malloc(_allocatedLength));
  if (!_buffer) {
    _allocatedLength = 0;
    vw_abort();
  }
  reset();
}

PacketVS2::~PacketVS2() {
  free(_buffer);
}

PacketVS2::operator bool() const {
  if (_buffer && _buffer[VS2_PACKET_LENGTH] != 0) return true;
  return false;
}

/*
uint8_t PacketVS2::operator[](std::size_t index) const {
  return _buffer[index];
}
*/

uint8_t& PacketVS2::operator[](std::size_t index) {
  return _buffer[index];
}

bool PacketVS2::createPacket(PacketType pt, FunctionCode fc, uint8_t id, uint16_t addr, uint8_t len, const uint8_t* data) {
  reset();

  // check arguments
  if (len == 0) {
    vw_log_w("Length error: %u", len);
    return false;
  }
  if (fc == FunctionCode::WRITE && !data) {
    vw_log_w("Function code - data mismatch");
    return false;
  }
  if (id > 7) {
    vw_log_w("Message id overflow: %u > 7", id);
  }

  // reserve memory
  std::size_t toAllocate = (fc == FunctionCode::WRITE) ? len + 6 : 6;
  if (toAllocate > _allocatedLength) {
    uint8_t* newBuffer = reinterpret_cast<uint8_t*>(realloc(_buffer, toAllocate));
    if (!newBuffer) {
      vw_log_e("buffer not available");
      return false;
    }
    _buffer = newBuffer;
    _allocatedLength = toAllocate;
  }

  // 2. Serialize into buffer
  if (fc == FunctionCode::WRITE) {
    _buffer[VS2_PACKET_LENGTH] = 0x05 + len;  // 0x05 = standard length: mt, fc, addr(2), len + data
  } else {
    _buffer[VS2_PACKET_LENGTH++] = 0x05;
  }
  _buffer[VS2_PACKET_TYPE] = static_cast<uint8_t>(pt);
  _buffer[VS2_FUNCTION_CODE_ID] = static_cast<uint8_t>(fc) | id << 5;
  _buffer[VS2_ADDRESS_HIGH] = (addr >> 8) & 0xFF;
  _buffer[VS2_ADDRESS_LOW] = addr & 0xFF;
  _buffer[VS2_DATA_LENGTH] = len;
  if (fc == FunctionCode::WRITE || pt == PacketType::RESPONSE) {
    for (uint8_t i = 0; i < len; ++i) {
      _buffer[VS2_DATA + i] = data[i];
    }
  }
  return true;
}

bool PacketVS2::setLength(uint8_t length) {
  std::size_t toAllocate = length + 1;
  if (toAllocate > _allocatedLength) {
    uint8_t* newBuffer = reinterpret_cast<uint8_t*>(realloc(_buffer, toAllocate));
    if (!newBuffer) {
      return false;
    }
    _allocatedLength = toAllocate;
    _buffer = newBuffer;
  }
  _buffer[0] = length;
  return true;
}

uint8_t PacketVS2::length() const {
  return _buffer[VS2_PACKET_LENGTH] + 1;
}

PacketType PacketVS2::packetType() const {
  return static_cast<PacketType>(_buffer[VS2_PACKET_TYPE]);
}

FunctionCode PacketVS2::functionCode() const {
  return static_cast<FunctionCode>(_buffer[VS2_FUNCTION_CODE_ID] & 0x1F);
}

uint8_t PacketVS2::id() const {
  return _buffer[VS2_FUNCTION_CODE_ID] >> 5 & 0x07;
}

uint16_t PacketVS2::address() const {
  uint16_t retVal = _buffer[VS2_ADDRESS_HIGH] << 8;
  retVal |= _buffer[VS2_ADDRESS_LOW];
  return retVal;
}

uint8_t PacketVS2::dataLength() const {
  return _buffer[VS2_DATA_LENGTH];
}

const uint8_t* PacketVS2::data() const {
  if (functionCode() == FunctionCode::WRITE) return nullptr;
  return &_buffer[VS2_DATA];
}

uint8_t PacketVS2::checksum() const {
  uint8_t retVal = 0;
  for (std::size_t i = 0; i <= _buffer[VS2_PACKET_LENGTH]; ++i) {
    retVal += _buffer[i];
  }
  return retVal;
}

void PacketVS2::reset() {
  _buffer[VS2_PACKET_LENGTH] = 0x00;
}

}  // end namespace VitoWiFi
