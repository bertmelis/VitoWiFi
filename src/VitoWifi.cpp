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

#include "VitoWifi.h"

VitoWifiBase::VitoWifiBase() :
  _enablePrinter(false),
  _printer(nullptr) {}

VitoWifiBase::~VitoWifiBase() {
  if (_enablePrinter && _printer)
    _printer->println(F("Destructing of VitoWifi is unsupported"));
  abort();
}

template <>
VitoWifiInterface<OptolinkP300>::VitoWifiInterface() {}
template <>
VitoWifiInterface<OptolinkKW>::VitoWifiInterface() {}

// For P300 Protocol
#ifdef ARDUINO_ARCH_ESP32
template <>
void VitoWifiInterface<OptolinkP300>::setup(HardwareSerial* serial, int8_t rxPin, int8_t txPin) {
  _optolink.begin(serial, rxPin, txPin);
}
#endif
#ifdef ESP8266
template <>
void VitoWifiInterface<OptolinkP300>::setup(HardwareSerial* serial) {
  _optolink.begin(serial);
}
#endif

// For KW Protocol
#ifdef ARDUINO_ARCH_ESP32
template <>
void VitoWifiInterface<OptolinkKW>::setup(HardwareSerial* serial, int8_t rxPin, int8_t txPin) {
  _optolink.begin(serial, rxPin, txPin);
  _datapoints.shrink_to_fit();
}
#endif
#ifdef ESP8266
template <>
void VitoWifiInterface<OptolinkKW>::setup(HardwareSerial* serial) {
  _optolink.begin(serial);
}
#endif

void VitoWifiBase::setGlobalCallback(Callback globalCallback) {
  if (_DPManager.size()) {
    _DPManager.at(0)->setGlobalCallback(globalCallback);
  }
}

IDatapoint& VitoWifiBase::addDatapoint(const char* name, const char* group, const uint16_t address, const DPType type, bool isWriteable) {
  return _DPManager.addDP(name, group, address, type, isWriteable);
}

IDatapoint& VitoWifiBase::addDatapoint(const char* name, const char* group, const uint16_t address, const DPType type) {
  return addDatapoint(name, group, address, type, false);
}

void VitoWifiBase::readAll() {
  for (auto it = _DPManager.begin(); it != _DPManager.end(); ++it) {
    _readDatapoint((*it).get());
  }
}

void VitoWifiBase::readGroup(const char* group) {
  for (auto it = _DPManager.begin(); it != _DPManager.end(); ++it) {
    if (strcmp(group, (*it).get()->getGroup()) == 0) {
      _readDatapoint((*it).get());
    }
  }
}

void VitoWifiBase::readDatapoint(const char* name) {
  for (auto it = _DPManager.begin(); it != _DPManager.end(); ++it) {
    if (strcmp(name, (*it).get()->getName()) == 0) {
      _readDatapoint((*it).get());
      return;
    }
  }
}

void VitoWifiBase::_readDatapoint(IDatapoint* dp) {
  Action action = {dp, false};
  _queue.push(action);
  if (_enablePrinter && _printer) {
    _printer->print("READ ");
    _printer->println(dp->getName());
  }
}

void VitoWifiBase::writeDatapoint(const char* name, DPValue value) {
  for (auto it = _DPManager.begin(); it != _DPManager.end(); ++it) {
    if (strcmp(name, (*it).get()->getName()) == 0) {
      _writeDatapoint((*it).get(), value);
      return;
    }
  }
}

void VitoWifiBase::_writeDatapoint(IDatapoint* dp, DPValue value) {
  if (!dp->isWriteable()) {
    if (_enablePrinter && _printer)
      _printer->println(F("DP is readonly, skipping"));
    return;
  }
  uint8_t value_enc[MAX_DP_LENGTH] = {0};
  dp->encode(value_enc, value);
  Action action = {dp, true};
  memcpy(action.value, value_enc, MAX_DP_LENGTH);
  _queue.push(action);
}

template <>
void VitoWifiInterface<OptolinkP300>::loop() {
  _optolink.loop();
  if (!_queue.empty() && !_optolink.isBusy()) {
    if (!_queue.front().write) {
      _optolink.readFromDP(_queue.front().DP->getAddress(), _queue.front().DP->getLength());
    } else {
      _optolink.writeToDP(_queue.front().DP->getAddress(), _queue.front().DP->getLength(), _queue.front().value);
    }
    return;
  }
  if (_optolink.available() > 0) {  // trigger callback when ready and remove element from queue
    if (_enablePrinter && _printer) {
      _printer->print(F("DP "));
      _printer->print(_queue.front().DP->getName());
      _printer->println(F(" succes"));
    }
    uint8_t value_enc[MAX_DP_LENGTH] = {0};
    _optolink.read(value_enc);
    _queue.front().DP->callback(_queue.front().DP->decode(value_enc));
    _queue.pop();
    return;
  }
  if (_optolink.available() < 0) {  // display error message and remove element from queue
    uint8_t errorCode = _optolink.readError();
    if (_enablePrinter && _printer) {
      _printer->print(F("DP "));
      _printer->print(_queue.front().DP->getName());
      _printer->print(F(" error: "));
      _printer->println(errorCode, DEC);
    }
    _queue.pop();
    return;
  }
}
template <>
void VitoWifiInterface<OptolinkKW>::loop() {
  _optolink.loop();
  if (!_queue.empty() && !_optolink.isBusy()) {
    if (!_queue.front().write) {
      _optolink.readFromDP(_queue.front().DP->getAddress(), _queue.front().DP->getLength());
    } else {
      _optolink.writeToDP(_queue.front().DP->getAddress(), _queue.front().DP->getLength(), _queue.front().value);
    }
    return;
  }
  if (_optolink.available() > 0) {  // trigger callback when ready and remove element from queue
    if (_enablePrinter && _printer) {
      _printer->print(F("DP "));
      _printer->print(_queue.front().DP->getName());
      _printer->println(F(" succes"));
    }
    uint8_t value_enc[MAX_DP_LENGTH] = {0};
    _optolink.read(value_enc);
    _queue.front().DP->callback(_queue.front().DP->decode(value_enc));
    _queue.pop();
    return;
  }
  if (_optolink.available() < 0) {  // display error message and remove element from queue
    uint8_t errorCode = _optolink.readError();
    if (_enablePrinter && _printer) {
      _printer->print(F("DP "));
      _printer->print(_queue.front().DP->getName());
      _printer->print(F(" error: "));
      _printer->println(errorCode, DEC);
    }
    _queue.pop();
    return;
  }
}

template <>
void VitoWifiInterface<OptolinkP300>::setLogger(Print* printer) {
  _printer = printer;
  _optolink.setLogger(_printer);
}
template <>
void VitoWifiInterface<OptolinkKW>::setLogger(Print* printer) {
  _printer = printer;
  _optolink.setLogger(_printer);
}
template <>
void VitoWifiInterface<OptolinkP300>::enableLogger() {
  _enablePrinter = true;
  _optolink.setLogger(_printer);
}
template <>
void VitoWifiInterface<OptolinkKW>::enableLogger() {
  _enablePrinter = true;
  _optolink.setLogger(_printer);
}
template <>
void VitoWifiInterface<OptolinkP300>::disableLogger() {
  _enablePrinter = false;
  _optolink.setLogger(nullptr);
}
template <>
void VitoWifiInterface<OptolinkKW>::disableLogger() {
  _enablePrinter = false;
  _optolink.setLogger(nullptr);
}
