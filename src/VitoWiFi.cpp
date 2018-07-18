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

#pragma once  // this file is #included in VitoWiFi.hpp to split templated interface from source, please ignore warning

#include "VitoWiFi.hpp"

template <class P>
VitoWiFiClass<P>::VitoWiFiClass() :
  _enablePrinter(false),
  _printer(nullptr) {}

template <class P>
VitoWiFiClass<P>::~VitoWiFiClass() {
  abort();  // destructing is not supported
}

#ifdef ARDUINO_ARCH_ESP32
template <class P>
void VitoWiFiClass<P>::setup(HardwareSerial* serial, int8_t rxPin, int8_t txPin) {
  _optolink.begin(serial, rxPin, txPin);
}
#endif
#ifdef ESP8266
template <class P>
void VitoWiFiClass<P>::setup(HardwareSerial* serial) {
  _optolink.begin(serial);
}
#endif

template <class P>
void VitoWiFiClass<P>::setGlobalCallback(Callback globalCallback) {
  IDatapoint::_dps.front()->setGlobalCallback(globalCallback);
}

template <class P>
void VitoWiFiClass<P>::readAll(void* arg) {
  for (auto it = IDatapoint::_dps.begin(); it != IDatapoint::_dps.end(); ++it) {
    _readDatapoint(*it, arg);
  }
}

template <class P>
void VitoWiFiClass<P>::readGroup(const char* group, void* arg) {
  for (auto it = IDatapoint::_dps.begin(); it != IDatapoint::_dps.end(); ++it) {
    if (strcmp(group, (*it)->getGroup()) == 0) {
      _readDatapoint(*it, arg);
    }
  }
}

template <class P>
void VitoWiFiClass<P>::readDatapoint(IDatapoint& dp, void* arg) {  // NOLINT TODO(@bertmelis) make it a const reference
  _readDatapoint(&dp, arg);
}

template <class P>
void VitoWiFiClass<P>::writeDatapoint(IDatapoint& dp, DPValue value, void* arg) {  // NOLINT TODO(@bertmelis) make it a const reference
  _writeDatapoint(&dp, value, arg);
}

template <class P>
void VitoWiFiClass<P>::_readDatapoint(IDatapoint* dp, void* arg) {
  if (_queue.size() < (IDatapoint::_dps.size() * 2)) {
    Action action = {dp, false, arg};
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
void VitoWiFiClass<P>::_writeDatapoint(IDatapoint* dp, DPValue value, void* arg) {
  if (!dp->isWriteable()) {
    if (_enablePrinter && _printer)
      _printer->println("DP is readonly, skipping");
    return;
  }
  if (_queue.size() < (IDatapoint::_dps.size() * 2)) {
    uint8_t value_enc[MAX_DP_LENGTH] = {0};
    dp->encode(value_enc, value);
    Action action = {dp, true, arg};
    memcpy(action.value, value_enc, MAX_DP_LENGTH);
    _queue.push(action);
  } else {
    if (_enablePrinter && _printer) {
      _printer->print("queue full");
    }
  }
}

template <class P>
void VitoWiFiClass<P>::loop() {
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
    _queue.front().DP->setValue(_queue.front().DP->decode(value_enc));
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
void VitoWiFiClass<P>::setLogger(Print* printer) {
  _printer = printer;
  _optolink.setLogger(_printer);
}

template <class P>
void VitoWiFiClass<P>::enableLogger() {
  _enablePrinter = true;
  _optolink.setLogger(_printer);
}

template <class P>
void VitoWiFiClass<P>::disableLogger() {
  _enablePrinter = false;
  _optolink.setLogger(nullptr);
}
