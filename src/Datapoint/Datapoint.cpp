/*
Copyright (c) 2023 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#include "Datapoint.h"

namespace VitoWiFi {

Datapoint::Datapoint(const char* name, uint16_t address, uint8_t length, const Converter& converter)
: _name(name)
, _address(address)
, _length(length)
, _converter(&converter) {
// empty
}

Datapoint::operator bool() const {
  if (_length == 0) return false;
  return true;
}

const char* Datapoint::name() const {
  return _name;
}

uint16_t Datapoint::address() const {
  return _address;
}

uint8_t Datapoint::length() const {
  return _length;
}

const Converter& Datapoint::converter() const {
  return *_converter;
}

VariantValue Datapoint::decode(const uint8_t* data, uint8_t length) const {
  return (*_converter).decode(data, length);
}

VariantValue Datapoint::decode(const PacketVS2& packet) const {
  return (*_converter).decode(packet.data(), packet.dataLength());
}

void Datapoint::encode(uint8_t* buf, uint8_t len, const VariantValue& value) const {
  return (*_converter).encode(buf, len, value);
}

}  // end namespace VitoWiFi
