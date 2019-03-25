/* VitoWiFi

Copyright 2019 Bert Melis

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#pragma once

#if defined ARDUINO_ARCH_ESP8266 || ARDUINO_ARCH_ESP32

#include "Datapoint.h"
#include <math.h>  // floor()

class DPCountS : public Datapoint {
 public:
  DPCountS(const char* name, const uint16_t address);
  ~DPCountS();
  void onData(std::function<void(uint16_t)> callback);
  void decode(uint8_t* data, uint8_t length, Datapoint* dp = nullptr) override;
  void encode(uint8_t* raw, uint8_t length, void* data) override;
  void encode(uint8_t* raw, uint8_t length, uint16_t data);

 private:
  std::function<void(uint16_t)> _onData;
};

#endif  // ARDUINO_ARCH_ESP8266 || ARDUINO_ARCH_ESP32
