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

*/

#pragma once
#include <Arduino.h>
#include "Constants.hpp"

class OptolinkP300 {
 public:
  OptolinkP300();
#ifdef ARDUINO_ARCH_ESP32
  void begin(HardwareSerial* serial, int8_t rxPin, int8_t txPin);
#endif
#ifdef ESP8266
  void begin(HardwareSerial* serial);
#endif
  void loop();
  const bool connected() const;
  const int8_t available() const;
  const bool isBusy() const;
  bool readFromDP(uint16_t address, uint8_t length);
  bool writeToDP(uint16_t address, uint8_t length, uint8_t value[]);
  void read(uint8_t value[]);
  const uint8_t readError();
  void setLogger(Print* printer);

 private:
  Stream* _stream;
  enum OptolinkState : uint8_t { RESET = 0, RESET_ACK, INIT, INIT_ACK, IDLE, SEND, SEND_ACK, RECEIVE, RECEIVE_ACK } _state;
  enum OptolinkAction : uint8_t { WAIT = 0, PROCESS, RETURN, RETURN_ERROR } _action;
  uint16_t _address;
  uint8_t _length;
  bool _writeMessageType;
  uint8_t _value[4];
  uint8_t _rcvBuffer[12];
  uint8_t _rcvBufferLen;
  uint8_t _rcvLen;
  uint32_t _lastMillis;
  uint8_t _errorCode;
  void _resetHandler();
  void _resetAckHandler();
  void _initHandler();
  void _initAckHandler();
  void _idleHandler();
  void _sendHandler();
  void _sendAckHandler();
  void _receiveHandler();
  void _receiveAckHandler();
  void _returnHandler();
  bool _transmit(uint16_t address, uint8_t length, bool write, uint8_t value[]);
  inline uint8_t _calcChecksum(uint8_t array[], uint8_t length);
  inline bool _checkChecksum(uint8_t array[], uint8_t length);
  inline void _printHex(Print* printer, uint8_t array[], uint8_t length);
  inline void _clearInputBuffer();
  Print* _printer;

  inline void _setState(OptolinkState state) {
    /*
    if (_printer) {
      _printer->print("Optolink state: ");
      _printer->println(static_cast<uint8_t>(state));
    }
    */
    _state = state;
  }
  inline void _setAction(OptolinkAction action) {
    /*
    if (_printer) {
      _printer->print("Optolink action: ");
      _printer->println(static_cast<uint8_t>(action));
    }
    */
    _action = action;
  }
};
