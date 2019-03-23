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

#include "Datapoint.h"

std::function<void(uint8_t[], uint8_t, Datapoint* dp)> Datapoint::_stdOnData = nullptr;

Datapoint::Datapoint(const char* name, uint16_t address, uint8_t length) :
  _name(name),
  _address(address),
  _length(length) {}

Datapoint::~Datapoint() {
  // nothing to do
}

void Datapoint::onData(std::function<void(uint8_t[], uint8_t, Datapoint* dp)> callback) {
  _stdOnData = callback;
}

const char* Datapoint::getName() const {
  return _name;
}

const uint16_t Datapoint::getAddress() const {
  return _address;
}

const uint8_t Datapoint::getLength() const {
  return _length;
}

void Datapoint::encode(uint8_t* raw, uint8_t length, void* data) {
  if (length != _length) {
    // display error about length
    memset(raw, 0, _length);
  } else {
      memcpy(raw, data, length);
  }
}

void Datapoint::decode(uint8_t* data, uint8_t length, Datapoint* dp) {
  uint8_t* output = new uint8_t[_length];
  memset(output, 0, _length);
  if (length != _length) {
    // display error about length
  } else {
    memcpy(output, data, length);
    if (_stdOnData) _stdOnData(output, _length, dp);
  }
  delete[] output;
}
