/*
Copyright (c) 2023 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#include "PacketGWG.h"

namespace VitoWiFi {

PacketGWG::PacketGWG()
: _allocatedLength(START_PAYLOAD_LENGTH + 5)
, _buffer(nullptr) {
  _buffer = reinterpret_cast<uint8_t*>(malloc(_allocatedLength));
  if (!_buffer) {
    _allocatedLength = 0;
    vw_abort();
  }
  reset();
}

PacketGWG::~PacketGWG() {
  free(_buffer);
}

PacketGWG::operator bool() const {
  if (_buffer && _buffer[3] != 0) return true;
  return false;
}

/*
uint8_t PacketGWG::operator[](std::size_t index) const {
  return _buffer[index];
}
*/

uint8_t& PacketGWG::operator[](std::size_t index) {
  return _buffer[index];
}

bool PacketGWG::createPacket(uint8_t packetType, uint16_t addr, uint8_t len, const uint8_t* data) {
  reset();

  // check arguments
  if (len == 0) {
    vw_log_w("Zero length given");
    return false;
  }
  if (addr > 0xFF) {
    vw_log_w("GWG doesn't support addresses > 0xFF");
    return false;
  }
  if (packetType != PacketGWGType.READ && packetType != PacketGWGType.WRITE) {
    vw_log_w("Packet type error: 0x%02x", packetType);
    return false;
  }
  if (packetType == PacketGWGType.WRITE && !data) {
    vw_log_w("No data for write packet");
    return false;
  }

  // reserve memory
  std::size_t toAllocate = (packetType == PacketGWGType.WRITE) ? len + 5 : 5;
  if (toAllocate > _allocatedLength) {
    uint8_t* newBuffer = reinterpret_cast<uint8_t*>(realloc(_buffer, toAllocate));
    if (!newBuffer) {
      return false;
    }
    _buffer = newBuffer;
    _allocatedLength = toAllocate;
  }

  // 2. Serialize into buffer
  size_t step = 0;
  _buffer[step++] = VitoWiFiInternals::ProtocolBytes.ENQ_ACK;
  _buffer[step++] = packetType;
  _buffer[step++] = addr & 0xFF;
  _buffer[step++] = len;
  if (packetType == PacketGWGType.WRITE) {
    for (uint8_t i = 0; i < len; ++i) {
      _buffer[step++] = data[i];
    }
  }
  _buffer[step] = VitoWiFiInternals::ProtocolBytes.EOT;
  return true;
}

uint8_t PacketGWG::length() const {
  if (_buffer[3] == 0) return 0;
  if (_buffer[1] == PacketGWGType.READ) return 5;
  if (_buffer[1] == PacketGWGType.WRITE) return _buffer[3] + 5;
  return 0;  // should not be possible
}

uint8_t PacketGWG::packetType() const {
  return _buffer[1];
}

uint16_t PacketGWG::address() const {
  return _buffer[2];
}

uint8_t PacketGWG::dataLength() const {
  return _buffer[3];
}

const uint8_t* PacketGWG::data() const {
  return &_buffer[4];
}

void PacketGWG::reset() {
  _buffer[3] = 0x00;
}

}  // end namespace VitoWiFi
