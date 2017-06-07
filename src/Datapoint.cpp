#include "Datapoint.h"

//declare static global callback function as part of the Datapoint base class
GlobalCallbackFunction Datapoint::_globalCallback = nullptr;
Print* Datapoint::_debugPrinter = nullptr;

Datapoint::Datapoint(const char* name, const char* group, const uint16_t address, bool isWriteable):
  _name(name),
  _group(group),
  _address(address),
  _writeable(isWriteable)
  {}


Datapoint::~Datapoint() {
  abort();  //destruction is not supported
}


const char* Datapoint::getName() const {
  return _name;
}


const char* Datapoint::getGroup() const {
  return _group;
}


const uint16_t Datapoint::getAddress() const {
  return _address;
}


const uint8_t Datapoint::getLength() const {
  return 0;
}


const bool Datapoint::isWriteable() const {
  return _writeable;
}


Datapoint& Datapoint::setWriteable() {
  _writeable = true;
  return *this;
}


void Datapoint::setGlobalCallback(GlobalCallbackFunction globalCallback) {
  _globalCallback = globalCallback;
}


void Datapoint::setDebugPrinter(Print* printer) {
  _debugPrinter = printer;
}


TempDP::TempDP(const char* name, const char* group, const uint16_t address, bool isWriteable):
  Datapoint(name, group, address, isWriteable),
  _callback(nullptr)
  {}


Datapoint& TempDP::setCallback(TempCallbackFunction callback) {
  _callback = callback;
  return *this;
}


void TempDP::callback(uint8_t value[]) {
  int16_t tmp = value[1] << 8 | value[0];
  float floatValue = (float)(tmp) / 10.0;
  _debugPrinter->println(floatValue, 1);
  if (_callback) {
    _debugPrinter->print(F("Calling callback of "));
    _debugPrinter->print(_group);
    _debugPrinter->print(F(" - "));
    _debugPrinter->print(_name);
    _debugPrinter->print(F(" with value "));
    _debugPrinter->print(floatValue, 1);
    _callback(_name, _group, floatValue);
  }
  else if (Datapoint::_globalCallback) {
    char str[5] = {'\0'};
    //snprintf(str, sizeof str, "%f", floatValue);
    dtostrf(floatValue, 3, 1, str);
    _debugPrinter->print(F("Calling global callback of "));
    _debugPrinter->print(_group);
    _debugPrinter->print(F(" - "));
    _debugPrinter->print(_name);
    _debugPrinter->print(F(" with value "));
    _debugPrinter->print(str);
    _globalCallback(_name, _group, str);
  }
  return;
}


void TempDP::transformValue(uint8_t transformedValue[], float value) {
  //to do: think about this conversion
  int16_t tmp = floor((value * 10) + 0.5);
  transformedValue[1] = tmp >> 8;
  transformedValue[0] = tmp & 0xFF;
  return;
}


StatDP::StatDP(const char* name, const char* group, const uint16_t address, bool isWriteable):
  Datapoint(name, group, address, isWriteable)
  {}


Datapoint& StatDP::setCallback(StatCallbackFunction callback) {
  _callback = callback;
  return *this;
}


void StatDP::callback(uint8_t value[]) {
  bool boolValue = (value[0]) ? true : false;
  if (_callback) {
    _debugPrinter->print(F("Calling callback of "));
    _debugPrinter->print(_group);
    _debugPrinter->print(F(" - "));
    _debugPrinter->print(_name);
    _debugPrinter->print(F(" with value "));
    _debugPrinter->print(boolValue, BIN);
    _callback(_name, _group, boolValue);
  }
  else if (_globalCallback) {
    const char* str = (boolValue) ? "1" : "0";  //or "true/false"?
    _debugPrinter->print(F("Calling global callback of "));
    _debugPrinter->print(_group);
    _debugPrinter->print(F(" - "));
    _debugPrinter->print(_name);
    _debugPrinter->print(F(" with value "));
    _debugPrinter->print(str);
    _globalCallback(_name, _group, str);
  }
}


void StatDP::transformValue(uint8_t transformedValue[], bool value) {
  transformedValue[0] = (value) ? 0x01 : 0x00;
  return;
}


/*
ModeDP::ModeDP(const char* name, const char* group, const uint16_t address, bool isWriteable):
  Datapoint(name, group, address, isWriteable)
  {}


Datapoint& ModeDP::setCallback(ModeCallbackFunction callback) {
  _callback = callback;
  return *this;
}


void ModeDP::callback(DPValue value) {
  uint8_t intValue = (value.byte1Value);
  if (_callback) {
    _callback(this->getName(), this->getGroup(), intValue);
  }
  else if (_globalCallback) {
    _globalCallback(getName(), getGroup(), intValue);
  }
  else {
    _debugPrinter->println(F("no callback found"));
  }
}


HoursDP::HoursDP(const char* name, const char* group, const uint16_t address, bool isWriteable):
  Datapoint(name, group, address, isWriteable)
  {}


Datapoint& HoursDP::setCallback(HoursCallbackFunction callback) {
  _callback = callback;
  return *this;
}


HoursLDP::HoursLDP(const char* name, const char* group, const uint16_t address, bool isWriteable):
  Datapoint(name, group, address, isWriteable)
  {}


Datapoint& HoursLDP::setCallback(HoursLCallbackFunction callback) {
  _callback = callback;
  return *this;
}


CountDP::CountDP(const char* name, const char* group, const uint16_t address, bool isWriteable):
  Datapoint(name, group, address, isWriteable)
  {}


Datapoint& CountDP::setCallback(CountCallbackFunction callback) {
  _callback = callback;
  return *this;
}
*/
