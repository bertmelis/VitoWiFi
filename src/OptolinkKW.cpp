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

#include "OptolinkKW.h"

OptolinkKW::OptolinkKW() :
    _stream(nullptr),
    _state(INIT),
    _action(WAIT),
    _address(0),
    _length(0),
    _writeMessageType(false),
    _value{0},
    _rcvBuffer{0},
    _rcvBufferLen(0),
    _rcvLen(0),
    _lastMillis(0),
    _numberOfTries(5),
    _errorCode(0),
    _printer(nullptr) {}

#ifdef ARDUINO_ARCH_ESP32
void OptolinkKW::begin(HardwareSerial* serial, int8_t rxPin, int8_t txPin) {
  serial->begin(4800, SERIAL_8E2, rxPin, txPin);
  _stream = serial;
  // serial->flush();
}
#endif
#ifdef ESP8266
void OptolinkKW::begin(HardwareSerial* serial) {
  serial->begin(4800, SERIAL_8E2);
  _stream = serial;
  // serial->flush();
}
#endif

void OptolinkKW::loop() {
  if (_numberOfTries < 1) {
    _setState(IDLE);
    _setAction(RETURN_ERROR);
  } else if (_state == INIT) {
    _initHandler();
  } else if (_state == IDLE) {
    _idleHandler();
  } else if (_state == RECEIVE) {
    _receiveHandler();
  }
}

// reset new devices to KW state by sending 0x04.
void OptolinkKW::_initHandler() {
  if (_stream->available()) {
    if (_stream->peek() == 0x05) {
      _setState(IDLE);
      _idleHandler();
      if (_printer)
        _printer->println("INIT done.");
    } else {
      _stream->read();
    }
  } else {
    if (millis() - _lastMillis > 1000UL) {
      _lastMillis = millis();
      const uint8_t buff[] = {0x04};
      _stream->write(buff, 1);
    }
  }
}

// idle state, waiting for sync from Vito
void OptolinkKW::_idleHandler() {
  if (_stream->available()) {
    if (_stream->read() == 0x05) {
      _lastMillis = millis();
      if (_action == PROCESS) {
        _setState(SYNC);
        _syncHandler();
      }
    } else {
      // received something unexpected
    }
  } else if (_action == PROCESS && (millis() - _lastMillis < 10UL)) {  // don't wait for 0x05 sync signal, send directly after last request
    _setState(SEND);
    _sendHandler();
  } else if (millis() - _lastMillis > 10 * 1000UL) {
    _setState(IDLE);
    _errorCode = 1;
    --_numberOfTries;
  }
}

void OptolinkKW::_syncHandler() {
  const uint8_t buff[1] = {0x01};
  _stream->write(buff, 1);
  _setState(SEND);
  _sendHandler();
}

//
void OptolinkKW::_sendHandler() {
  uint8_t buff[6];
  if (_writeMessageType) {
    // type is WRITE
    // has length of 4 chars + length of value
    buff[0] = 0xF4;
    buff[1] = (_address >> 8) & 0xFF;
    buff[2] = _address & 0xFF;
    buff[3] = _length;
    // add value to message
    memcpy(&buff[4], _value, _length);
    _rcvLen = 1;  // expected length is only ACK (0x00)
    _stream->write(buff, 4 + _length);
  } else {
    // type is READ
    // has fixed length of 4 chars
    buff[0] = 0xF7;
    buff[1] = (_address >> 8) & 0xFF;
    buff[2] = _address & 0xFF;
    buff[3] = _length;
    _rcvLen = _length;  // expected answer length the same as sent
    _stream->write(buff, 4);
  }
  _clearInputBuffer();
  _rcvBufferLen = 0;
  --_numberOfTries;
  _setState(RECEIVE);
  if (_writeMessageType) {
    if (_printer)
      _printer->print(F("WRITE "));
  } else {
    if (_printer) {
      _printer->print(F("READ "));
      _printHex(_printer, &buff[1], 2);
      _printer->println();
    }
  }
}

void OptolinkKW::_receiveHandler() {
  while (_stream->available() > 0) {  // while instead of if: read complete RX buffer
    _rcvBuffer[_rcvBufferLen] = _stream->read();
    ++_rcvBufferLen;
  }
  if (_rcvBufferLen == _rcvLen) {  // message complete, TODO: check message (eg 0x00 for READ messages)
    _setState(IDLE);
    _setAction(RETURN);
    _lastMillis = millis();
    _errorCode = 0;  // succes
    if (_printer)
      _printer->println(F("ack"));
    return;
  } else if (millis() - _lastMillis > 2 * 1000UL) {  // Vitotronic isn't answering, try again
    _rcvBufferLen = 0;
    _errorCode = 1;  // Connection error
    memset(_rcvBuffer, 0, 4);
    _setState(IDLE);
    _setAction(RETURN_ERROR);
    if (_printer)
      _printer->println(F("nack"));
  }
}

// set properties for datapoint and move state to SEND
bool OptolinkKW::readFromDP(uint16_t address, uint8_t length) {
  if (_action != WAIT) {
    return false;
  }
  // setup properties for next state in communicationHandler
  _address = address;
  _length = length;
  _writeMessageType = false;
  _rcvBufferLen = 0;
  _numberOfTries = 5;
  memset(_rcvBuffer, 0, 4);
  _setAction(PROCESS);
  return true;
}

// set properties datapoint and move state to SEND
bool OptolinkKW::writeToDP(uint16_t address, uint8_t length, uint8_t value[]) {
  if (_action != WAIT) {
    return false;
  }
  // setup variables for next state
  _address = address;
  _length = length;
  memcpy(_value, value, _length);
  _writeMessageType = true;
  _rcvBufferLen = 0;
  _numberOfTries = 5;
  memset(_rcvBuffer, 0, 4);
  _setAction(PROCESS);
  return true;
}

const int8_t OptolinkKW::available() const {
  if (_action == RETURN_ERROR)
    return -1;
  else if (_action == RETURN)
    return 1;
  else
    return 0;
}

const bool OptolinkKW::isBusy() const {
  if (_action == WAIT)
    return false;
  else
    return true;
}

// return value and reset comunication to IDLE
void OptolinkKW::read(uint8_t value[]) {
  if (_action != RETURN) {
    return;
  }
  if (_writeMessageType) {  // return original value in case of WRITE command
    memcpy(value, &_value, _length);
    _setAction(WAIT);
    return;
  } else {
    memcpy(value, &_rcvBuffer, _length);
    _setAction(WAIT);
    return;
  }
}

const uint8_t OptolinkKW::readError() {
  _setAction(WAIT);
  return _errorCode;
}

// clear serial input buffer
inline void OptolinkKW::_clearInputBuffer() {
  while (_stream->available() > 0) {
    _stream->read();
  }
}

void OptolinkKW::setLogger(Print* printer) {
  _printer = printer;
}

// Copied from Arduino.cc forum --> (C) robtillaart
inline void OptolinkKW::_printHex(Print* printer, uint8_t array[], uint8_t length) {
  char tmp[length * 2 + 1];  // NOLINT
  byte first;
  uint8_t j = 0;
  for (uint8_t i = 0; i < length; ++i) {
    first = (array[i] >> 4) | 48;
    if (first > 57)
      tmp[j] = first + (byte)39;
    else
      tmp[j] = first;
    ++j;

    first = (array[i] & 0x0F) | 48;
    if (first > 57)
      tmp[j] = first + (byte)39;
    else
      tmp[j] = first;
    ++j;
  }
  tmp[length * 2] = 0;
  printer->print(tmp);
}
