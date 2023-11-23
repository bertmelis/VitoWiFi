/*
Copyright (c) 2023 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#pragma once

#ifdef ARDUINO

#include <cassert>

#include <HardwareSerial.h>

namespace VitoWiFiInternals {

class HardwareSerialInterface : public SerialInterface {
 public:
  explicit HardwareSerialInterface(HardwareSerial* interface)
  : _interface(interface) {
    assert(interface);
  }

  bool begin() {
    _interface->begin(4800, SERIAL_8E2);
    return (*_interface);
  }

  std::size_t write(const uint8_t* data, uint8_t length) override {
    return _interface->write(data, length);
  }

  uint8_t read() override {
    uint8_t retVal = 0;
    _interface->read(&retVal, 1);
    return retVal;
  }

  size_t available() override {
    return _interface->available();
  }

 private:
  HardwareSerial* _interface;
};

}  // end namespace VitoWiFiInternals

#endif
