/*
Copyright (c) 2023 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#pragma once

#if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)

#include <cassert>

#include <SoftwareSerial.h>
#include "SerialInterface.h"

namespace VitoWiFiInternals {

class SoftwareSerialInterface : public SerialInterface {
 public:
  explicit SoftwareSerialInterface(SoftwareSerial* interface);
  bool begin();
  void end();
  std::size_t write(const uint8_t* data, uint8_t length) override;
  uint8_t read() override;
  size_t available() override;

 private:
  SoftwareSerial* _interface;
};

}  // end namespace VitoWiFiInternals

#endif
