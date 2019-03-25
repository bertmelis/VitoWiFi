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
 * @file OptolinkP300.h
 * @brief Optolink API definions (protocol P300).
 *
 * This file contains all method definitions for the Optolink for the 
 * protocol P300.
 */

#pragma once

#if defined ARDUINO

#include "Optolink.h"
#include <Arduino.h>  // for millis

/**
 * @brief Base class for the Optolink (P300).
 * 
 * This class is a P300 version of the Optolink.
 */
class OptolinkP300 : public Optolink {
 public:
  /**
   * @brief Construct the Optolink object (P300)
   * 
   * @param serial Hardwareserial object to be used. Pass by reference.
   */
  explicit OptolinkP300(HardwareSerial* serial);
  ~OptolinkP300();

  /**
   * @brief Method to start the Optolink.
   * 
   * Calling this method starts the Hardwareserial (passed in the constructor)
   * with the right settings.
   */
  void begin();

  /**
   * @brief Method to keep the Optolink running.
   * 
   * This methid has to be called frequently. The Optolink object works by 
   * polling the Hardwareserial. If applicable, add a delay between calls to 
   * feed a watchdog (needed for ESP8266 and ESP32).
   */
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
};

#elif defined VITOWIFI_TEST

#else

#pragma message "no suitable platform"

#endif
