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

/**
 * @file VitoWiFi.h
 * @brief API definitions for VitoWiFi
 *
 * This library is made to use with the Arduino framework for 
 * esp8266 and esp32.
 */

#pragma once

#if defined ARDUINO_ARCH_ESP8266 || ARDUINO_ARCH_ESP32

#include "OptolinkP300.h"

#include "Datapoints/DPTemp.h"
#include "Datapoints/DPStat.h"
#include "Datapoints/DPCountS.h"

/**
 * @brief Protocols implemented in VitoWiFi
 * 
 * Possible values are
 * - P300: the newer protocol with connection management and message integrity 
 *         checks (checksum). Units are backward compatible with KW.
 * - KW: older protocol, based on the early GWG but with support for 
 *       datapoints > 2 bytes.
 * 
 * When first implementing VitoWiFi you should test with P300. If your heater 
 * doesn't respond switch to KW.
 * 
 */
enum VitoWiFiProtocol {
  P300,
  KW
};

class VitoWiFi {
 public:
  VitoWiFi(VitoWiFiProtocol protocol, HardwareSerial* serial) :
    _optolink(nullptr) {
      switch (protocol) {
      case P300:
        _optolink = new OptolinkP300(serial);
        break;
      case KW:
        // to be implemented
      default:
        abort();
      }
      if (_optolink) {
        _optolink->begin();
      }
  }
  ~VitoWiFi() {
    delete _optolink;
  }
  void onData(std::function<void(const uint8_t[], uint8_t, Datapoint* dp)> callback) {
    Datapoint::globalOnData(callback);
  }

  void onError(std::function<void(uint8_t error)> callback) {
    _optolink->onError(callback);
  }

  void begin() {
    _optolink->begin();
  }

  void loop() {
    _optolink->loop();
  }

  void readAll() {
    for (Datapoint* dp : Datapoint::_datapoints) {
      read(*dp);
    }
  }

  bool read(const Datapoint& datapoint) {
    return _optolink->read(datapoint.getAddress(), datapoint.getLength(), reinterpret_cast<void*>(&datapoint));
  }


  template<class D, typename T>
  bool write(const D& datapoint, T value) {
    uint8_t* raw = new uint8_t[datapoint->getLength()];  // temporary variable to hold encoded value, will be copied by optolink
    datapoint->encode(raw, datapoint->getLength(), value);
    return _optolink->write(datapoint->getAddress(), datapoint.getLength(), raw, reinterpret_cast<void*>(datapoint));
    delete[] raw;
  }

 private:
  Optolink* _optolink;
};

#elif defined VITOWIFI_TEST

#else

#pragma message "no suitable platform"

#endif
