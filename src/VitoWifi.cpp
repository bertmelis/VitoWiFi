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

#pragma once  // Please ignore this error.

#include "VitoWifi.h"

template <class P>
VitoWifiClass<P>::VitoWifiClass() :
  _enablePrinter(false),
  _printer(nullptr) {}

template <class P>
VitoWifiClass<P>::~VitoWifiClass() {
  abort();  // destructing is not supported
}

#ifdef ARDUINO_ARCH_ESP32
template <class P>
void VitoWifiClass<P>::setup(HardwareSerial* serial, int8_t rxPin, int8_t txPin) {
  _optolink.begin(serial, rxPin, txPin);
}
#endif
#ifdef ESP8266
template <class P>
void VitoWifiClass<P>::setup(HardwareSerial* serial) {
  _optolink.begin(serial);
}
#endif

template <class P>
void VitoWifiClass<P>::setGlobalCallback(Callback globalCallback) {
    _DPManager.executeAll([this, globalCallback](IDatapoint* dp){
      dp->setGlobalCallback(globalCallback);
    });
}

template <class P>
IDatapoint& VitoWifiClass<P>::addDatapoint(const char* name, const char* group, const uint16_t address, const DPType type, bool isWriteable) {
  return _DPManager.addDP(name, group, address, type, isWriteable);
}

template <class P>
IDatapoint& VitoWifiClass<P>::addDatapoint(const char* name, const char* group, const uint16_t address, const DPType type) {
  return addDatapoint(name, group, address, type, false);
}

template <class P>
void VitoWifiClass<P>::readAll() {
  _DPManager.executeAll([this](IDatapoint* dp){
    _readDatapoint(dp);
  });
}

template <class P>
void VitoWifiClass<P>::readGroup(const char* group) {
  _DPManager.executeGroup(group, [this](IDatapoint* dp) {
    _readDatapoint(dp);
  });
}

template <class P>
void VitoWifiClass<P>::readDatapoint(const char* name) {
  _DPManager.executeDP(name, [this](IDatapoint* dp) {
    _readDatapoint(dp);
  });
}

template <class P>
void VitoWifiClass<P>::writeDatapoint(const char* name, DPValue value) {
  _DPManager.executeDP(name, [this, value](IDatapoint* dp) {
    _writeDatapoint(dp, value);
  });
}

template <class P>
void VitoWifiClass<P>::_readDatapoint(IDatapoint* dp) {
  if (_queue.size() < (_DPManager.numberOfDPs() * 2)) {
    Action action = {dp, false};
    _queue.push(action);
    if (_enablePrinter && _printer) {
      _printer->print("READ ");
      _printer->println(dp->getName());
    }
  } else {
    if (_enablePrinter && _printer) {
      _printer->print("queue full");
    }
  }
}

template <class P>
void VitoWifiClass<P>::_writeDatapoint(IDatapoint* dp, DPValue value) {
  if (!dp->isWriteable()) {
    if (_enablePrinter && _printer)
      _printer->println("DP is readonly, skipping");
    return;
  }
  if (_queue.size() < (_DPManager.numberOfDPs() * 2)) {
    uint8_t value_enc[MAX_DP_LENGTH] = {0};
    dp->encode(value_enc, value);
    Action action = {dp, true};
    memcpy(action.value, value_enc, MAX_DP_LENGTH);
    _queue.push(action);
  } else {
    if (_enablePrinter && _printer) {
      _printer->print("queue full");
    }
  }
}

template <class P>
void VitoWifiClass<P>::loop() {
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
      _printer->print("DP ");
      _printer->print(_queue.front().DP->getName());
      _printer->println(" succes");
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
      _printer->print("DP ");
      _printer->print(_queue.front().DP->getName());
      _printer->print(" error: ");
      _printer->println(errorCode, DEC);
    }
    _queue.pop();
    return;
  }
}

template <class P>
void VitoWifiClass<P>::setLogger(Print* printer) {
  _printer = printer;
  _optolink.setLogger(_printer);
}

template <class P>
void VitoWifiClass<P>::enableLogger() {
  _enablePrinter = true;
  _optolink.setLogger(_printer);
}

template <class P>
void VitoWifiClass<P>::disableLogger() {
  _enablePrinter = false;
  _optolink.setLogger(nullptr);
}
