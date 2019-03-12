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

#pragma once

#if defined ARDUINO_ARCH_ESP8266 || ARDUINO_ARCH_ESP32

#ifndef VITOWIFI_MAX_QUEUE_LENGTH
  #define VITOWIFI_MAX_QUEUE_LENGTH 20
#endif
#ifndef MAX_DP_LENGTH
#define MAX_DP_LENGTH 9
#endif

#include <functional>
#include <queue>
#include <HardwareSerial.h>
#include <string.h>  // for memcpy

enum OptolinkError : uint8_t {
  TIMEOUT,
  LENGTH,
  NACK,
  CRC,
  VITO_ERROR
};

struct Optolink_DP {
  uint16_t address;
  uint8_t length;
  bool write;
  uint8_t* data;
  void* arg;
  Optolink_DP(uint16_t address, uint8_t length, bool write, uint8_t* value, void* arg);
  ~Optolink_DP();
};

class Optolink {
 public:
  explicit Optolink(HardwareSerial* serial);
  virtual ~Optolink();
  void onData(std::function<void(uint8_t* data, uint8_t len, void* arg)> callback);
  void onError(std::function<void(uint8_t error)> callback);
  bool read(uint16_t address, uint8_t length, void* arg = nullptr);
  bool write(uint16_t address, uint8_t length, uint8_t* data, void* arg = nullptr);

  virtual void begin() = 0;
  virtual void loop() = 0;


 protected:
  HardwareSerial* _serial;
  std::queue<Optolink_DP> _queue;  // TODO(bertmelis): add semaphore to ESP32 version to guard access to queue?
  std::function<void(uint8_t* data, uint8_t len, void* arg)> _onData;
  std::function<void(uint8_t error)> _onError;
};

#elif defined VITOWIFI_TEST

#else

#pragma message "no suitable platform"

#endif
