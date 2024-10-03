/*
Copyright (c) 2023 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#pragma once

#include <cstdint>
#include <cstddef>

#ifndef VW_START_PAYLOAD_LENGTH
#define VW_START_PAYLOAD_LENGTH 10
#endif

namespace VitoWiFi {

constexpr size_t START_PAYLOAD_LENGTH = VW_START_PAYLOAD_LENGTH;

enum class FunctionCode : uint8_t {
  READ  = 0x01,
  WRITE = 0x02,
  RPC   = 0x07,
};

enum class PacketType : uint8_t {
  REQUEST  = 0x00,
  RESPONSE = 0x01,
  UNACKED  = 0x02,
  ERROR    = 0x03,
};

constexpr struct {
  uint8_t READ  = 0xF7;
  uint8_t WRITE = 0xF4;
} PacketVS1Type;

constexpr struct {
  uint8_t READ  = 0xCB;
  uint8_t WRITE = 0xC8;
} PacketGWGType;

enum class OptolinkResult {
  CONTINUE,
  PACKET,
  TIMEOUT,
  LENGTH,
  NACK,
  CRC,
  ERROR
};

}  // end namespace VitoWiFi

namespace VitoWiFiInternals {

constexpr struct {
  uint8_t PACKETSTART = 0x41;
  uint8_t ACK         = 0x06;
  uint8_t ENQ_ACK     = 0x01;
  uint8_t NACK        = 0x15;
  uint8_t ENQ         = 0x05;
  uint8_t EOT         = 0x04;
  uint8_t SYNC[3]     = {0x16, 0x00, 0x00};
} ProtocolBytes;

enum class ParserResult {
  CONTINUE,
  COMPLETE,
  CS_ERROR,
  ERROR
};

}  // end namespace VitoWiFiInternals
