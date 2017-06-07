#include "VitoWifi.h"

//create an instance of the VitoWifiClass to be used in the main sketch
VitoWifiClass VitoWifi;


VitoWifiClass::VitoWifiClass():
  _optolink(),
  _enableLed(false),
  _blinker(),
  _logger() {
    _optolink.setDebugPrinter(&_logger);
}


VitoWifiClass::~VitoWifiClass() {
  _logger.println(F("Destructing of VitoWifi is unsupported"));
  abort();
}


//pass serial to Optolink
void VitoWifiClass::setup(HardwareSerial& serial) {
  setup(serial);
}
void VitoWifiClass::setup(HardwareSerial* serial) {
  //pass serial to OptoLink
  _optolink.begin(serial);
  //_blinker.start(1);
  _logger.println(F("VitoWifi: Setup...done"));
}


void VitoWifiClass::setGlobalCallback(GlobalCallbackFunction globalCallback) {
  if (_datapoints.size()) {
    _datapoints.front()->setGlobalCallback(globalCallback);
  }
}


Datapoint& VitoWifiClass::addDatapoint(const char* name, const char* group, const uint16_t address, const DPType type) {
  switch (type) {
    case TEMP :
    {
      Datapoint* tempDP = new TempDP(name, group, address, false);
      tempDP->setDebugPrinter(&_logger);
      _datapoints.push_back(tempDP);
      break;
    }
    case STAT :
    {
      Datapoint* statDP = new StatDP(name, group, address, false);
      statDP->setDebugPrinter(&_logger);
      _datapoints.push_back(statDP);
      break;
    }
  }
  return *_datapoints.back();
}


/*
Datapoint& VitoWifiClass::addDatapoint(const char* name, const char* group, const uint16_t address, const DPType type, bool isWriteable) {
  _datapoints.push_back(Datapoint(name, group, address, isWriteable));
  return _datapoints.back();
}
*/


Datapoint* VitoWifiClass::getDatapoint(const char* name) {
  //find DP and return reference
  Datapoint* DP = nullptr;
  for (uint8_t i = 0; i < _datapoints.size(); i++) {
    if (strcmp(name, _datapoints.at(i)->getName()) == 0)
      DP = _datapoints.at(i);
  }
  return DP;
}


void VitoWifiClass::readAll() {
  bool foundOne = false;
  for (Datapoint* iDP : _datapoints) {
    Action action = {iDP, false};
    _queue.push(action);
    _logger.print(F("Datapoint "));
    _logger.print(iDP->getName());
    _logger.println(F(" READ action added."));
    foundOne = true;
  }
  if (foundOne) return;
  _logger.println(F("No datapoints available, skipping"));
  return;
}


void VitoWifiClass::readGroup(const char* group) {
  bool foundOne = false;
  for (Datapoint* iDP : _datapoints) {
    if (strcmp(group, iDP->getGroup()) == 0) {
      Action action = {iDP, false};
      _queue.push(action);
      _logger.print(F("Datapoint "));
      _logger.print(iDP->getName());
      _logger.println(F(" READ action added."));
      foundOne = true;
    }
  }
  if(!foundOne) _logger.println(F("Group not found, skipping"));
}


void VitoWifiClass::readDatapoint(const char* name) {
  for (Datapoint* iDP : _datapoints) {
    if (strcmp(name, iDP->getName()) == 0) {
      Action action = {iDP, false};
      _queue.push(action);
      _logger.print(F("Datapoint "));
      _logger.print(iDP->getName());
      _logger.println(F(" READ action added."));
      return;
    }
  }
  _logger.println(F("Datapoint not found, skipping"));
  return;
}


/* Writing bool is not supported as there's no DP that uses this (known to me)
void VitoWifiClass::writeDatapoint(const char* name, bool value) {
  Datapoint* DP = getDatapoint(name);
  if (DP) {
    if (DP->getLength() != 1) {
      _logger.println(F("Wrong \"writeDatapoint\" method, skipping."));
      return;
    }
    Action action;
    action.DP = DP;
    action.write = true;
    action.value[0] = (value) ? 1 : 0;
    _queue.push(action);
    return;
  }
  _logger.println(F("Datapoint not found, skipping"));
  return;
}
*/


void VitoWifiClass::writeDatapoint(const char* name, uint8_t value) {
  Datapoint* DP = getDatapoint(name);
  if (DP) {
    if (DP->getLength() != 1) {
      _logger.println(F("Wrong \"writeDatapoint\" method, skipping."));
      return;
    }
    Action action;
    action.DP = DP;
    action.write = true;
    action.value[0] = value;
    _queue.push(action);
    return;
  }
  _logger.println(F("Datapoint not found, skipping"));
  return;
}


void VitoWifiClass::writeDatapoint(const char* name, float value) {
  Datapoint* DP = getDatapoint(name);
  if (DP) {
    if (DP->getLength() != 2) {
      _logger.println(F("Wrong \"writeDatapoint\" method, skipping."));
      return;
    }
    uint8_t transformedValue[] = {0};
    DP->transformValue(transformedValue, value);
    Action action;
    action.DP = DP;
    action.write = true;
    memcpy(action.value, transformedValue, 2);
    _queue.push(action);
    return;
  }
  _logger.println(F("Datapoint not found, skipping"));
  return;
}


void VitoWifiClass::loop(){
  _optolink.loop();
  if (_queue.empty()) return;  //do nothing when queue is empty
  if (!_isBusy) {  //do something but not when busy
    if (_queue.front().write) {
      _optolink.writeToDP(_queue.front().DP->getAddress(), _queue.front().DP->getLength(), _queue.front().value);
      _isBusy = true;
    }
    else {
      _optolink.readFromDP(_queue.front().DP->getAddress(), _queue.front().DP->getLength());
      _isBusy = true;
    }
  }
  //trigger callback when ready and remove element from queue
  if (_optolink.available() > 0) {
    _logger.println("Value available, fetching.");
    uint8_t value[4] = {0};
    _optolink.read(value);
    _queue.front().DP->callback(value);
    _queue.pop();
    _isBusy = false;
  }
  if (_optolink.available() < 0) {
    uint8_t value[4] = {0};
    _optolink.read(value);
    _logger.print(F("Datapoint "));
    _logger.print(_queue.front().DP->getName());
    _logger.println(F(" action unsuccesfull. Skipped."));
    _queue.pop();
    _isBusy = false;
  }
}


void VitoWifiClass::enableLed(uint8_t pin, uint8_t on) {
 _enableLed = true;
 _blinker.setLedPin(pin, on);
}


//Logger stuff, taken from Marvin ROGER's Homie for ESP8266
void VitoWifiClass::setLoggingPrinter(Print* printer) {
  _logger.setPrinter(printer);
}
void VitoWifiClass::enableLogger() {
  _logger.setLogging(true);
}
void VitoWifiClass::disableLogger() {
  _logger.setLogging(false);
}
/*
Logger& VitoWifiClass::getLogger() {
  return _logger;
}
*/
