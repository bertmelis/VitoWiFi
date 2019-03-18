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

#include "DPCountS.h"

DPCountS::DPCountS(const char* name, const uint16_t address) :
  Datapoint(name, address, 2),
  _onData(nullptr) {}

DPCountS::~DPCountS() {
  // empty
}

void DPCountS::onData(std::function<void(uint16_t)> callback) {
  _onData = callback;
}

void DPCountS::decode(uint8_t* data, uint8_t length) {
  assert(length >= _length);
  if (_onData) {
    uint16_t output = data[1] << 8 | data[0];
    _onData(output);
  } else {
    Datapoint::decode(data, length, this);
  }
}

void DPCountS::encode(uint8_t* raw, uint8_t length, uint16_t data) {
  assert(length >= _length);
  raw[1] = data >> 8;
  raw[0] = data & 0xFF;
}
