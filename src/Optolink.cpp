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

#include "Optolink.h"

#if defined ARDUINO_ARCH_ESP8266 || ARDUINO_ARCH_ESP32

Optolink::Optolink(HardwareSerial* serial) :
  _serial(serial),
  _queue(VITOWIFI_MAX_QUEUE_LENGTH),
  _onData(nullptr),
  _onError(nullptr) {}

Optolink::~Optolink() {
  // nothing to do
}

void Optolink::onData(void (*callback)(uint8_t* data, uint8_t len)) {
  _onData = reinterpret_cast<OnDataArgCallback>(callback);
}

void Optolink::onData(OnDataArgCallback callback) {
  _onData = callback;
}

void Optolink::onError(void (*callback)(uint8_t error)) {
  _onError = reinterpret_cast<OnErrorArgCallback>(callback);
}

void Optolink::onError(OnErrorArgCallback callback) {
  _onError = callback;
}

bool Optolink::read(uint16_t address, uint8_t length, void* arg) {
  OptolinkDP dp(address, length, false, nullptr, arg);
  return _queue.push(dp);
}

bool Optolink::write(uint16_t address, uint8_t length, uint8_t* data, void* arg) {
  OptolinkDP dp(address, length, true, data, arg);
  return _queue.push(dp);
}

void Optolink::_tryOnData(uint8_t* data, uint8_t len) {
  if (_onData) _onData(data, len, _queue.front()->arg);
  _queue.pop();
}

void Optolink::_tryOnError(uint8_t error) {
  if (_onError) _onError(error, _queue.front()->arg);
  _queue.pop();
}

#elif defined VITOWIFI_TEST

#else

#pragma message "no suitable platform"

#endif
