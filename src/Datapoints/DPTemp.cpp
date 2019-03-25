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

#if defined ARDUINO_ARCH_ESP8266 || ARDUINO_ARCH_ESP32

#include "DPTemp.h"

DPTemp::DPTemp(const char* name, const uint16_t address) :
  Datapoint(name, address, 2),
  _onData(nullptr) {}

DPTemp::~DPTemp() {
  // empty
}

void DPTemp::onData(std::function<void(float)> callback) {
  _onData = callback;
}

void DPTemp::decode(uint8_t* data, uint8_t length, Datapoint* dp) {
  assert(length >= _length);
  if (!dp) dp = this;
  int16_t tmp = data[1] << 8 | data[0];
  float value = tmp / 10.0f;
  if (_onData) {
    _onData(value);
  } else {
    Datapoint::decode(data, length, dp);
  }
}

void DPTemp::encode(uint8_t* raw, uint8_t length, void* data) {
  float value = *reinterpret_cast<float*>(data);
  encode(raw, length, value);
}

void DPTemp::encode(uint8_t* raw, uint8_t length, float data) {
  assert(length >= _length);
  int16_t tmp = floor((data * 10) + 0.5);
  raw[1] = tmp >> 8;
  raw[0] = tmp & 0xFF;
}

#endif  // ARDUINO_ARCH_ESP8266 || ARDUINO_ARCH_ESP32
