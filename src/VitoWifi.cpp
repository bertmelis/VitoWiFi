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
  _datapoints.shrink_to_fit();
}
#endif
#ifdef ESP8266
template <>
void VitoWifiInterface<OptolinkP300>::setup(HardwareSerial* serial) {
  _optolink.begin(serial);
  _datapoints.shrink_to_fit();
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
  _datapoints.shrink_to_fit();
}
#endif

void VitoWifiBase::setGlobalCallback(GlobalCallbackFunction globalCallback) {
  if (_datapoints.size()) {
    _datapoints.front()->setGlobalCallback(globalCallback);
  }
}

Datapoint& VitoWifiBase::addDatapoint(const char* name, const char* group, const uint16_t address, const DPType type, bool isWriteable) {
  Datapoint* DP;
  switch (type) {
    case TEMP:
      DP = new TempDP(name, group, address, isWriteable);
      break;
    case TEMPS:
      DP = new TempSDP(name, group, address, isWriteable);
      break;
    case STAT:
      DP = new StatDP(name, group, address, isWriteable);
      break;
    case COUNT:
      DP = new CountDP(name, group, address, isWriteable);
      break;
    case COUNTS:
      DP = new CountSDP(name, group, address, isWriteable);
      break;
    case MODE:
      DP = new ModeDP(name, group, address, isWriteable);
      break;
    case COP:
      DP = new COPDP(name, group, address, isWriteable);
      break;
  }
  if (!DP) abort();  // out of memory?
  _datapoints.push_back(DP);
  return *_datapoints.back();
}

Datapoint& VitoWifiBase::addDatapoint(const char* name, const char* group, const uint16_t address, const DPType type) { return addDatapoint(name, group, address, type, false); }

Datapoint* VitoWifiBase::_getDatapoint(const char* name) {
  Datapoint* DP = nullptr;
  for (std::vector<Datapoint*>::iterator it = _datapoints.begin(); it != _datapoints.end(); ++it) {
    if (strcmp(name, (*it)->getName()) == 0) DP = *it;
  }
  return DP;
}

void VitoWifiBase::readAll() {
  for (Datapoint* iDP : _datapoints) {
    _readDatapoint(iDP);
  }
}

void VitoWifiBase::readGroup(const char* group) {
  for (Datapoint* iDP : _datapoints) {
    if (strcmp(group, iDP->getGroup()) == 0) {
      _readDatapoint(iDP);
    }
  }
}

void VitoWifiBase::readDatapoint(const char* name) {
  for (Datapoint* iDP : _datapoints) {
    if (strcmp(name, iDP->getName()) == 0) {
      _readDatapoint(iDP);
      return;
    }
  }
}

inline void VitoWifiBase::_readDatapoint(Datapoint* dp) {
  Action action = {dp, false};
  _queue.push(action);
  if (_enablePrinter && _printer) {
    _printer->print("READ ");
    _printer->println(dp->getName());
  }
}

void VitoWifiBase::_writeDatapoint(const char* name, float value, size_t length) {
  Datapoint* DP = _getDatapoint(name);
  if (DP) {
    if (DP->getLength() != length) {
      if (_enablePrinter && _printer)
        _printer->println(F("Value does not match DP, skipping"));
      return;
    }
    if (!DP->isWriteable()) {
      if (_enablePrinter && _printer)
        _printer->println(F("DP is readonly, skipping"));
      return;
    }
    uint8_t transformedValue[2] = {0};
    DP->parse(transformedValue, value);
    Action action;
    action.DP = DP;
    action.write = true;
    memcpy(action.value, transformedValue, 2);
    _queue.push(action);
    return;
  }
}

template <>
void VitoWifiInterface<OptolinkP300>::loop() {
  _optolink.loop();
  if (!_queue.empty() && !_optolink.isBusy()) {
    if (_queue.front().write) {
      _optolink.writeToDP(_queue.front().DP->getAddress(), _queue.front().DP->getLength(), _queue.front().value);
    } else {
      _optolink.readFromDP(_queue.front().DP->getAddress(), _queue.front().DP->getLength());
    }
    return;
  }
  if (_optolink.available() > 0) {  // trigger callback when ready and remove element from queue
    if (_enablePrinter && _printer) {
      _printer->print(F("DP "));
      _printer->print(_queue.front().DP->getName());
      _printer->println(F(" succes"));
    }
    uint8_t value[4] = {0};
    _optolink.read(value);
    _queue.front().DP->callback(value);
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
    if (_queue.front().write) {
      _optolink.writeToDP(_queue.front().DP->getAddress(), _queue.front().DP->getLength(), _queue.front().value);
    } else {
      _optolink.readFromDP(_queue.front().DP->getAddress(), _queue.front().DP->getLength());
    }
    return;
  }
  if (_optolink.available() > 0) {  // trigger callback when ready and remove element from queue
    if (_enablePrinter && _printer) {
      _printer->print(F("DP "));
      _printer->print(_queue.front().DP->getName());
      _printer->println(F(" succes"));
    }
    uint8_t value[4] = {0};
    _optolink.read(value);
    _queue.front().DP->callback(value);
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
