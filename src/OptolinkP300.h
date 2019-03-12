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

#include "Optolink.h"
#include <Arduino.h>  // for millis

class OptolinkP300 : public Optolink {
  public:
    OptolinkP300(HardwareSerial* serial);
    ~OptolinkP300();
    void begin();
    void loop();

  private:
    enum OptolinkState : uint8_t {
      RESET = 0,
      RESET_ACK,
      INIT,
      INIT_ACK,
      IDLE,
      SEND,
      SEND_ACK,
      RECEIVE,
      RECEIVE_ACK,
      UNDEF
    } _state; 
    void _reset();
    void _resetAck();
    void _init();
    void _initAck();
    void _idle();
    void _send();
    void _sentAck();
    void _receive();
    void _receiveAck();
    uint32_t _lastMillis;
    bool _write;
    uint8_t _rcvBuffer[MAX_DP_LENGTH + 8];
    size_t _rcvBufferLen;
    size_t _rcvLen;

    inline void _tryOnData(uint8_t* data, uint8_t len, void* arg);
    inline void _tryOnError(uint8_t error);
};

#elif defined VITOWIFI_TEST

#else

#pragma message "no suitable platform"

#endif
