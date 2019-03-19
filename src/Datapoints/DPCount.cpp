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

#include "DPCount.h"

DPCount::DPCount(const char* name, const uint16_t address) :
  Datapoint(name, address, 4),
  _onData(nullptr) {}

DPCount::~DPCount() {
  // empty
}

void DPCount::onData(std::function<void(uint32_t)> callback) {
  _onData = callback;
}

void DPCount::decode(const uint8_t* data, const uint8_t length, Datapoint* dp) {
  assert(length >= _length);
  if (!dp) dp = this;
  if (_onData) {
    uint32_t output = data[3] << 24 | data[2] << 16 | data[1] << 8 | data[0];
    _onData(output);
  } else {
    Datapoint::decode(data, length, dp);
  }
}

void DPCount::encode(uint8_t* raw, const uint8_t length, const void* data) {
  encode(raw, length, *reinterpret_cast<const uint32_t*>(data));
}

void DPCount::encode(uint8_t* raw, const uint8_t length, const uint32_t data) {
  assert(length >= _length);
  raw[3] = data >> 24;
  raw[2] = data >> 16;
  raw[1] = data >> 8;
  raw[0] = data & 0xFF;
}
