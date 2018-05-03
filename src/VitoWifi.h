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

Using portions or complete code from:
Hex print: 2011, robtillaart @ Arduino.cc forum

BIG thanks to https://github.com/openv/openv
and many others

*/

#pragma once
#include <Arduino.h>
#include <queue>
#include "Constants.h"
#include "Datapoint.h"
#include "OptolinkKW.h"
#include "OptolinkP300.h"

class VitoWifiBase {
 public:
  VitoWifiBase();
  ~VitoWifiBase();

  void loop();
  void setGlobalCallback(Callback globalCallback);
  IDatapoint& addDatapoint(const char* name, const char* group, const uint16_t address, const DPType type, bool isWriteable);
  IDatapoint& addDatapoint(const char* name, const char* group, const uint16_t address, const DPType type);

  void readAll();
  void readGroup(const char* group);
  void readDatapoint(const char* name);

  void writeDatapoint(const char* name, DPValue value);

  DPManager& getDPManager() { return _DPManager; }

 protected:
 void _readDatapoint(IDatapoint* DP);
 void _writeDatapoint(IDatapoint* DP, DPValue value);
  struct Action {
    IDatapoint* DP;
    bool write;
    uint8_t value[MAX_DP_LENGTH];
  };
  DPManager _DPManager;
  std::queue<Action> _queue;
  bool _enablePrinter;
  Print* _printer;
};

template <class P>
class VitoWifiInterface : public VitoWifiBase {
 public:
  VitoWifiInterface() {}
  ~VitoWifiInterface() {}
#ifdef ARDUINO_ARCH_ESP32
  void setup(HardwareSerial* serial, int8_t rxPin, int8_t txPin);
#endif
#ifdef ESP8266
  void setup(HardwareSerial* serial);
#endif
  void loop();

  void enableLogger();
  void disableLogger();
  void setLogger(Print* printer);

 private:
  P _optolink;
};

#define P300 OptolinkP300
#define KW OptolinkKW
#define VitoWifi_setProtocol(protocol) VitoWifiInterface<protocol> VitoWifi
