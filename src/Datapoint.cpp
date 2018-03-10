/*

Copyright 2017 Bert Melis

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

// declare static global callback function as part of the Datapoint base class
GlobalCallbackFunction Datapoint::_globalCallback = nullptr;

Datapoint::Datapoint(const char* name, const char* group, const uint16_t address, bool isWriteable) :
  _name(name),
  _group(group),
  _address(address),
  _writeable(isWriteable) {}

Datapoint::~Datapoint() {
  abort();  // destruction is not supported
}

Datapoint& Datapoint::setWriteable() {
  _writeable = true;
  return *this;
}

Datapoint& Datapoint::setGlobalCallback(GlobalCallbackFunction globalCallback) {
  _globalCallback = globalCallback;
  return *this;
}

TempDP::TempDP(const char* name, const char* group, const uint16_t address, bool isWriteable) :
  Datapoint(name, group, address, isWriteable),
  _callback(nullptr) {}

Datapoint& TempDP::setCallback(TempCallbackFunction callback) {
  _callback = callback;
  return *this;
}

void TempDP::callback(uint8_t value[]) {
  int16_t tmp = value[1] << 8 | value[0];
  float floatValue = static_cast<float>(tmp) / 10.0;
  if (_callback) {
    _callback(_name, _group, floatValue);
  } else if (Datapoint::_globalCallback) {
    char str[6] = {'\0'};
    dtostrf(floatValue, 3, 1, str);
    _globalCallback(_name, _group, str);
  }
  return;
}

void TempDP::parse(uint8_t transformedValue[], float value) {
  int16_t tmp = floor((value * 10) + 0.5);
  transformedValue[1] = tmp >> 8;
  transformedValue[0] = tmp & 0xFF;
  return;
}

TempSDP::TempSDP(const char* name, const char* group, const uint16_t address, bool isWriteable) :
  Datapoint(name, group, address, isWriteable),
  _callback(nullptr) {}

Datapoint& TempSDP::setCallback(TempSCallbackFunction callback) {
  _callback = callback;
  return *this;
}

void TempSDP::callback(uint8_t value[]) {
  if (_callback) {
    _callback(_name, _group, value[0]);
  } else if (Datapoint::_globalCallback) {
    char str[4] = {'\0'};
    snprintf(str, sizeof(str), "%u", value[0]);
    _globalCallback(_name, _group, str);
  }
  return;
}

void TempSDP::parse(uint8_t transformedValue[], float value) {
  transformedValue[0] = static_cast<uint8_t>(value);
  return;
}

StatDP::StatDP(const char* name, const char* group, const uint16_t address, bool isWriteable) :
  Datapoint(name, group, address, isWriteable),
  _callback(nullptr) {}

Datapoint& StatDP::setCallback(StatCallbackFunction callback) {
  _callback = callback;
  return *this;
}

void StatDP::callback(uint8_t value[]) {
  bool boolValue = (value[0]) ? true : false;
  if (_callback) {
    _callback(_name, _group, boolValue);
  } else if (_globalCallback) {
    char str[2] = {'\0'};
    snprintf(str, sizeof(str), "%d", boolValue);
    _globalCallback(_name, _group, str);
  }
}

void StatDP::parse(uint8_t transformedValue[], float value) {
  transformedValue[0] = (value) ? 0x01 : 0x00;
  return;
}

CountDP::CountDP(const char* name, const char* group, const uint16_t address, bool isWriteable) :
  Datapoint(name, group, address, isWriteable),
  _callback(nullptr) {}

Datapoint& CountDP::setCallback(CountCallbackFunction callback) {
  _callback = callback;
  return *this;
}

void CountDP::callback(uint8_t value[]) {
  uint32_t ui32 = value[0] | (value[1] << 8) | (value[2] << 16) | (value[3] << 24);
  if (_callback) {
    _callback(_name, _group, ui32);
  } else if (_globalCallback) {
    char str[11] = {'\0'};
    snprintf(str, sizeof(str), "%" PRIu32, ui32);
    _globalCallback(_name, _group, str);
  }
}

void CountDP::parse(uint8_t transformedValue[], float value) {
  uint32_t _value = (uint32_t)ceil(value);
  transformedValue[3] = _value >> 24;
  transformedValue[2] = _value >> 16;
  transformedValue[1] = _value >> 8;
  transformedValue[0] = _value & 0xFF;
  return;
}

CountSDP::CountSDP(const char* name, const char* group, const uint16_t address, bool isWriteable) :
  Datapoint(name, group, address, isWriteable),
  _callback(nullptr) {}

Datapoint& CountSDP::setCallback(CountSCallbackFunction callback) {
  _callback = callback;
  return *this;
}

void CountSDP::callback(uint8_t value[]) {
  uint16_t retValue = value[1] << 8 | value[0];
  if (_callback) {
    _callback(_name, _group, retValue);
  } else if (Datapoint::_globalCallback) {
    char str[6] = {'\0'};
    snprintf(str, sizeof(str), "%u", retValue);
    _globalCallback(_name, _group, str);
  }
  return;
}

void CountSDP::parse(uint8_t transformedValue[], float value) {
  uint16_t _value = (uint16_t)ceil(value);
  transformedValue[1] = _value >> 8;
  transformedValue[0] = _value & 0xFF;
  return;
}

ModeDP::ModeDP(const char* name, const char* group, const uint16_t address, bool isWriteable) :
  Datapoint(name, group, address, isWriteable),
  _callback(nullptr) {}

Datapoint& ModeDP::setCallback(ModeCallbackFunction callback) {
  _callback = callback;
  return *this;
}

void ModeDP::callback(uint8_t value[]) {
  if (_callback) {
    _callback(_name, _group, value[0]);
  } else if (_globalCallback) {
    char str[4] = {'\0'};  // 4 instead of 2 to allow reuse DP for setting temp to 1-byte-temp-dp
                           // TODO(@bertmelis): add and rearrange DPs
    snprintf(str, sizeof(str), "%u", value[0]);
    _globalCallback(_name, _group, str);
  }
}

void ModeDP::parse(uint8_t transformedValue[], float value) {
  transformedValue[0] = static_cast<uint8_t>(value);
  return;
}
