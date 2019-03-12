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

#include "OptolinkKW.h"

#if defined ARDUINO_ARCH_ESP8266 || ARDUINO_ARCH_ESP32

inline void clearInput(HardwareSerial* serial) {
  while (serial->read()) {}
}

OptolinkKW::OptolinkKW(HardwareSerial* serial) :
  Optolink(serial) {}

OptolinkKW::~OptolinkKW() {
  // TODO(bertmelis): anything to do?
}

void OptolinkKW::begin() {
  _serial->begin(4800, SERIAL_8E2);
  _state = RESET;
}

void OptolinkKW::loop() {
  // TODO(bertmelis)
}

#elif defined VITOWIFI_TEST

#else

#pragma message "no suitable platform"

#endif
