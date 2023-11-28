/*
Copyright (c) 2023 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)

#include "HardwareSerialInterface.h"

namespace VitoWiFiInternals {
HardwareSerialInterface::HardwareSerialInterface(HardwareSerial* interface)
: _interface(interface) {
  assert(interface);
}

bool HardwareSerialInterface::begin() {
  _interface->begin(4800, SERIAL_8E2);
  return (*_interface);
}

void HardwareSerialInterface::end() {
  #if defined(ARDUINO_ARCH_ESP32)
  _interface->end(true);
  #else
  _interface->end();
  #endif
}

std::size_t HardwareSerialInterface::write(const uint8_t* data, uint8_t length) {
  return _interface->write(data, length);
}

uint8_t HardwareSerialInterface::read() {
  uint8_t retVal = 0;
  _interface->read(&retVal, 1);
  return retVal;
}

size_t HardwareSerialInterface::available() {
  return _interface->available();
}

}  // end namespace VitoWiFiInternals

#endif
