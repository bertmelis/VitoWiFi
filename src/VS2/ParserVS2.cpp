/*
Copyright (c) 2023 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#include "ParserVS2.h"

namespace VitoWiFiInternals {

ParserVS2::ParserVS2()
: _packet()
, _step(ParserStep::STARTBYTE)
, _payloadLength(0) {
  // empty
}

ParserResult ParserVS2::parse(const uint8_t b) {
  switch (_step) {
  case ParserStep::STARTBYTE:
    if (b != ProtocolBytes.PACKETSTART) {
      vw_log_w("Invalid packet start: 0x%02x", b);
      break;
    }
    _packet.reset();
    _step = ParserStep::PACKETLENGTH;
    break;

  case ParserStep::PACKETLENGTH:
    if (b < 5) {
      vw_log_w("Invalid packet length: %u", b);
      _step = ParserStep::STARTBYTE;
      return ParserResult::ERROR;
    }
    if (!_packet.setLength(b)) {
      vw_log_e("Could not parse packet");
      _step = ParserStep::STARTBYTE;
      return ParserResult::ERROR;
    }
    _step = ParserStep::PACKETTYPE;
    break;

  case ParserStep::PACKETTYPE:
    if (b > 0x03) {
      vw_log_w("Invalid packet type: 0x%02x", b);
      _step = ParserStep::STARTBYTE;
      return ParserResult::ERROR;
    }
    _packet[1] = b;
    _step = ParserStep::FLAGS;
    break;

  case ParserStep::FLAGS:
    {
    uint8_t fc = b & 0x1F;
    if (fc != 0x01 && fc != 0x02 && fc != 0x07) {
      vw_log_w("Invalid packet fc: 0x%02x", fc);
      _step = ParserStep::STARTBYTE;
      return ParserResult::ERROR;
    }
    }
    _packet[2] = b;
    _step = ParserStep::ADDRESS1;
    break;

  case ParserStep::ADDRESS1:
    _packet[3] = b;
    _step = ParserStep::ADDRESS2;
    break;

  case ParserStep::ADDRESS2:
    _packet[4] = b;
    _step = ParserStep::PAYLOADLENGTH;
    break;

  case ParserStep::PAYLOADLENGTH:
    _packet[5] = b;
    if (_packet.functionCode() == VitoWiFi::FunctionCode::WRITE ||
        _packet.packetType() == VitoWiFi::PacketType::RESPONSE) {
      if (b != _packet.length() - 6U) {
        vw_log_w("Invalid payload length: %u (expected %u)", b, _packet.length() - 6U);
        _step = ParserStep::STARTBYTE;
        return ParserResult::ERROR;
      }
      _payloadLength = b;
      _step = ParserStep::PAYLOAD;
    } else {
      _step = ParserStep::CHECKSUM;
    }
    break;

  case ParserStep::PAYLOAD:
    _packet[6 + _packet.dataLength() - _payloadLength--] = b;
    if (_payloadLength == 0) {
      _step = ParserStep::CHECKSUM;
    }
    break;

  case ParserStep::CHECKSUM:
    if (_packet.checksum() != b) {
      vw_log_w("Invalid checksum: 0x%02x (calculated 0x%02x)", b, _packet.checksum());
      return ParserResult::CS_ERROR;
    }
    _step = ParserStep::STARTBYTE;
    return ParserResult::COMPLETE;
  }
  return ParserResult::CONTINUE;
}

const VitoWiFi::PacketVS2& ParserVS2::packet() const {
  return _packet;
}

}  // end namespace VitoWiFiInternals
