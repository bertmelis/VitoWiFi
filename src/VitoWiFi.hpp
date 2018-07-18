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
#include "Constants.hpp"
#include "Datapoint.hpp"
#include "OptolinkKW.hpp"
#include "OptolinkP300.hpp"

template <class P>
class VitoWiFiClass {
 public:
  VitoWiFiClass();
  ~VitoWiFiClass();
#ifdef ARDUINO_ARCH_ESP32
  void setup(HardwareSerial* serial, int8_t rxPin, int8_t txPin);
#endif
#ifdef ESP8266
  void setup(HardwareSerial* serial);
#endif
  void loop();
  void setGlobalCallback(Callback globalCallback);
  void readAll(void* arg = nullptr);
  void readGroup(const char* group, void* arg = nullptr);
  void readDatapoint(IDatapoint& dp, void* arg = nullptr);  // NOLINT TODO(bertmelis) make it a const reference
  void writeDatapoint(IDatapoint& dp, DPValue value, void* arg = nullptr);  // NOLINT TODO(bertmelis) make it a const reference
  void enableLogger();
  void disableLogger();
  void setLogger(Print* printer);

 private:
  struct Action {
    IDatapoint* DP;
    bool write;
    void* arg;
    uint8_t value[MAX_DP_LENGTH];
  };
  void _readDatapoint(IDatapoint* dp, void* arg);
  void _writeDatapoint(IDatapoint* dp, DPValue value, void* arg);
  P _optolink;
  std::queue<Action> _queue;
  bool _enablePrinter;
  Print* _printer;
};

#include "VitoWiFi.cpp"

#define P300 OptolinkP300
#define KW OptolinkKW
#define VitoWiFi_setProtocol(protocol) VitoWiFiClass<protocol> VitoWiFi
