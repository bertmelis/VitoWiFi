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

#if defined ARDUINO_ARCH_ESP8266 || ARDUINO_ARCH_ESP32 || VITOWIFI_TEST

#include "DPCop.h"

DPCop::DPCop(const char* name, const uint16_t address) :
  Datapoint(name, address, 1),
  _onData(nullptr) {}

DPCop::~DPCop() {
  // empty
}

void DPCop::onData(std::function<void(float)> callback) {
  _onData = callback;
}

void DPCop::decode(uint8_t* data, uint8_t length, Datapoint* dp) {
  assert(length >= _length);
  if (!dp) dp = this;
  if (_onData) {
    float output = data[0] / 10.0f;
    _onData(output);
  } else {
    Datapoint::decode(data, length, dp);
  }
}

void DPCop::encode(uint8_t* raw, uint8_t length, void* data) {
  float value = *reinterpret_cast<float*>(data);
  encode(raw, length, value);
}

void DPCop::encode(uint8_t* raw, uint8_t length, float data) {
  assert(length >= _length);
  raw[0] = floor((data * 10) + 0.5);
}

#endif  // ARDUINO_ARCH_ESP8266 || ARDUINO_ARCH_ESP32 || VITOWIFI_TEST
