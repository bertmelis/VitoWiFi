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
  Optolink(serial),
  _state(RESET),
  _lastMillis(0),
  _rcvBuffer{0},
  _rcvBufferLen(0),
  _rcvLen(0) {}

void OptolinkKW::begin() {
  _serial->begin(4800, SERIAL_8E2);
  _state = RESET;
}

void OptolinkKW::loop() {
  // TODO(bertmelis)
}

void OptolinkKW::_init() {
  if (_serial->available()) {
    if (_serial->peek() == 0x05) {
      _state = IDLE;
      _idle();
    } else {
      _serial->read();
    }
  } else {
    if (millis() - _lastMillis > 1000UL) {  // reset should Vitotronic be
                                            // connected (P300)
      _lastMillis = millis();
      const uint8_t buff[] = {0x04};
      _serial->write(buff, sizeof(buff));
    }
  }
}

void OptolinkKW::_idle() {
  if (_serial->available()) {
    if (_serial->read() == 0x05) {
      _lastMillis = millis();
      if (_queue.size() > 0) {
        _state = SYNC;
      }
    } else {
      // received something unexpected
    }
  } else if (_queue.size() > 0 && (millis() - _lastMillis < 10UL)) {
    // don't wait for 0x05 sync signal, send directly after last request
    _state = SEND;
  } else if (millis() - _lastMillis > 5 * 1000UL) {
    _state = INIT;
  }
}

void OptolinkKW::_sync() {
  const uint8_t buff[1] = {0x01};
  _serial->write(buff, sizeof(buff));
  _state = SEND;
  _send();
}

//
void OptolinkKW::_send() {
  uint8_t buff[MAX_DP_LENGTH + 4];
  OptolinkDP* dp = _queue.front();
  uint8_t length = dp->length;
  uint16_t address = dp->address;
  if (dp->write) {
    // type is WRITE
    // has length of 4 chars + length of value
    buff[0] = 0xF4;
    buff[1] = (address >> 8) & 0xFF;
    buff[2] = address & 0xFF;
    buff[3] = length;
    // add value to message
    memcpy(&buff[4], dp->data, length);
    _rcvLen = 1;  // expected length is only ACK (0x00)
    _serial->write(buff, 4 + length);
  } else {
    // type is READ
    // has fixed length of 4 chars
    buff[0] = 0xF7;
    buff[1] = (address >> 8) & 0xFF;
    buff[2] = address & 0xFF;
    buff[3] = length;
    _rcvLen = length;  // expected answer length the same as sent
    _serial->write(buff, 4);
  }
  _rcvBufferLen = 0;
  _state = RECEIVE;
  _lastMillis = millis();
}

void OptolinkKW::_receive() {
  while (_serial->available() > 0) {  // while instead of if: read complete RX buffer
    _rcvBuffer[_rcvBufferLen] = _serial->read();
    ++_rcvBufferLen;
  }
  if (_rcvBufferLen == _rcvLen) {  // message complete, TODO: check message (eg 0x00 for READ messages)
    _state = IDLE;
    _lastMillis = millis();
    if (_queue.front()->write) {
      _tryOnData(_queue.front()->data, _queue.front()->length);
    } else {
      _tryOnData(_rcvBuffer, _rcvBufferLen);
    }
    return;
  } else if (millis() - _lastMillis > 1 * 1000UL) {  // Vitotronic isn't answering, try again
    _tryOnError(TIMEOUT);
    _rcvBufferLen = 0;
    memset(_rcvBuffer, 0, MAX_DP_LENGTH);
    _state = INIT;
  }
}

#elif defined VITOWIFI_TEST

#else

#pragma message "no suitable platform"

#endif
