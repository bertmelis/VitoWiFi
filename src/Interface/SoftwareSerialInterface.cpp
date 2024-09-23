/*
Copyright (c) 2023 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#if defined(ARDUINO_ARCH_ESP8266)

#include "SoftwareSerialInterface.h"

namespace VitoWiFiInternals {

SoftwareSerialInterface::SoftwareSerialInterface(SoftwareSerial* interface)
: _interface(interface) {
  assert(interface);
}

bool SoftwareSerialInterface::begin() {
  _interface->begin(4800, EspSoftwareSerial::SWSERIAL_8E2);
  return (*_interface);
}

void SoftwareSerialInterface::end() {
  _interface->end();
}

std::size_t SoftwareSerialInterface::write(const uint8_t* data, uint8_t length) {
  return _interface->write(data, length);
}

uint8_t SoftwareSerialInterface::read() {
  uint8_t retVal = 0;
  _interface->read(&retVal, 1);
  return retVal;
}

size_t SoftwareSerialInterface::available() {
  return _interface->available();
}

}  // end namespace VitoWiFiInternals

#endif
