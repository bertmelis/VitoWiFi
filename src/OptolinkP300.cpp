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

#include "OptolinkP300.h"

OptolinkP300::OptolinkP300() :
    _stream(nullptr),
    _address(0),
    _length(0),
    _value{0},
    _writeMessageType(false),
    _rcvBuffer{0},
    _rcvBufferLen(0),
    _rcvLen(0),
    _debugMessage(true),
    _state(RESET),
    _action(PROCESS),
    _lastMillis(0),
    _numberOfTries(5),
    _errorCode(0),
    _logger() {}

// begin serial @ 4800 baud, 8 bits, even parity, 2 stop bits
#ifdef ARDUINO_ARCH_ESP32
void OptolinkP300::begin(HardwareSerial* serial, int8_t rxPin, int8_t txPin) {
  serial->begin(4800, SERIAL_8E2, rxPin, txPin);
  _stream = serial;
  // serial->flush();
}
#endif
#ifdef ESP8266
void OptolinkP300::begin(HardwareSerial* serial) {
  serial->begin(4800, SERIAL_8E2);
  _stream = serial;
  // serial->flush();
}
#endif

void OptolinkP300::loop() {
  if (_numberOfTries < 1) {
    setState(IDLE);
    setAction(RETURN_ERROR);
  }
  switch (_state) {
    case RESET:
      _resetHandler();
      break;
    case RESET_ACK:
      _resetAckHandler();
      break;
    case INIT:
      _initHandler();
      break;
    case INIT_ACK:
      _initAckHandler();
      break;
    case IDLE:
      _idleHandler();
      break;
    case SEND:
      _sendHandler();
      break;
    case SEND_ACK:
      _sendAckHandler();
      break;
    case RECEIVE:
      _receiveHandler();
      break;
    case RECEIVE_ACK:
      _receiveAckHandler();
      break;
  }
}

// Set communication with Vitotronic to defined state = reset to KW protocol
void OptolinkP300::_resetHandler() {
  const uint8_t buff[] = {0x04};
  _stream->write(buff, sizeof(buff));
  _lastMillis = millis();
  setState(RESET_ACK);
  if (_debugMessage) {
    _logger.println(F("Resetting Optolink..."));
    _debugMessage = false;
  }
}

void OptolinkP300::_resetAckHandler() {
  if (_stream->available()) {
    if (_stream->peek() == 0x05) {  // use peek so connection can be made immediately in next state
      // received 0x05/enquiry: optolink has been reset
      setState(INIT);
      _debugMessage = true;
      _logger.println(F("Optolink reset."));
    } else {
      _clearInputBuffer();
    }
  } else {
    if (millis() - _lastMillis > 500) {  // try again every 0,5sec
      setState(RESET);
    }
  }
}

// send initiator to Vitotronic to establish connection
void OptolinkP300::_initHandler() {
  if (_debugMessage) {
    _logger.println(F("Establishing Optolink connection..."));
    _debugMessage = false;
  }
  if (_stream->available()) {
    if (_stream->read() == 0x05) {
      // 0x05/ENQ received, sending initiator
      const uint8_t buff[] = {0x16, 0x00, 0x00};
      _stream->write(buff, sizeof(buff));
      _lastMillis = millis();
      setState(INIT_ACK);
    }
  }
}

void OptolinkP300::_initAckHandler() {
  if (_stream->available()) {
    if (_stream->read() == 0x06) {
      // ACK received, moving to next state
      setState(IDLE);
      setAction(WAIT);
      _debugMessage = true;
      // debug: done
      _logger.println(F("Optolink connection established."));
    } else {
      // return to previous state
      _clearInputBuffer();
      setState(INIT);
    }
  }
  if (millis() - _lastMillis > 10 * 1000UL) {  // if no ACK is coming, reset connection
    setState(RESET);
  }
}

// idle state, waiting for user action
void OptolinkP300::_idleHandler() {
  if (millis() - _lastMillis > 2 * 60 * 1000UL) {  // send INIT every 2 minutes to keep communication alive
    setState(INIT);
  }
  _clearInputBuffer();  // keep input clean
  if (_action == PROCESS) {
    setState(SEND);
  }
}

void OptolinkP300::_sendHandler() {
  uint8_t buff[12];
  if (_writeMessageType) {
    // type is WRITE
    // has length of 8 chars + length of value
    buff[0] = 0x41;
    buff[1] = 5 + _length;
    buff[2] = 0x00;
    buff[3] = 0x02;
    buff[4] = (_address >> 8) & 0xFF;
    buff[5] = _address & 0xFF;
    buff[6] = _length;
    // add value to message
    memcpy(&buff[7], _value, _length);
    buff[7 + _length] = _calcChecksum(buff, 8 + _length);
    _stream->write(buff, 8 + _length);

    // The return is always 8 byte long apparently
    // This is mentioned here: https://openv.wikispaces.com/Protokoll+300
    // At the bottom of the page (look for: RX: Data: 0x41 0x05 0x01 0x02 0x23 0x23 0x01 0x4f )
    _rcvLen = 8;
  } else {
    // type is READ
    // has fixed length of 8 chars
    buff[0] = 0x41;
    buff[1] = 0x05;
    buff[2] = 0x00;
    buff[3] = 0x01;
    buff[4] = (_address >> 8) & 0xFF;
    buff[5] = _address & 0xFF;
    buff[6] = _length;
    buff[7] = _calcChecksum(buff, 8);
    _rcvLen = 8 + _length;  // expected answer length is 8 + data length
    _stream->write(buff, 8);
  }
  _rcvBufferLen = 0;
  _lastMillis = millis();
  --_numberOfTries;
  setState(SEND_ACK);
  if (_writeMessageType) {
    _logger.print(F("WRITE "));
    _printHex(&_logger, buff, 8 + _length);
  } else {
    _logger.print(F("READ "));
    _printHex(&_logger, buff, 8);
  }
  _logger.print(F("... "));
}

void OptolinkP300::_sendAckHandler() {
  if (_stream->available()) {
    uint8_t buff = _stream->read();
    if (buff == 0x06) {  // transmit succesful, moving to next state
      _logger.println(F("ack"));
      setState(RECEIVE);
      return;
    } else if (buff == 0x15) {  // transmit negatively acknowledged, return to INIT and try again
      _logger.println(F("nack"));
      setState(INIT);
      _clearInputBuffer();
      return;
    }
  }
  if (millis() - _lastMillis > 2 * 1000UL) {  // if no ACK is coming, return to INIT and try again
    _logger.println(F("t/o"));
    setState(INIT);
    _clearInputBuffer();
  }
}

void OptolinkP300::_receiveHandler() {
  while (_stream->available() > 0) {  // while instead of if: read complete RX buffer
    _rcvBuffer[_rcvBufferLen] = _stream->read();
    ++_rcvBufferLen;
  }

  _logger.print(F("received: "));
  _printHex(&_logger, _rcvBuffer, _rcvBufferLen);

  if (_rcvBuffer[0] != 0x41) return;  // TODO(@bertmelis): find out why this is needed! I'd expect the rx-buffer to be empty.

  if (_rcvBufferLen == _rcvLen) {          // message complete, check message
    if (_rcvBuffer[1] != (_rcvLen - 3)) {  // check for message length
      _numberOfTries = 0;
      _errorCode = 4;
      _logger.println(F("... message length error"));
      return;
    }
    if (_rcvBuffer[2] != 0x01) {  // Vitotronic returns an error message, skipping DP
      _numberOfTries = 0;
      _errorCode = 3;  // Vitotronic error
      _logger.println(F("... Vitotronic error"));
      return;
    }
    if (!_checkChecksum(_rcvBuffer, _rcvLen)) {  // checksum is wrong, trying again
      _rcvBufferLen = 0;
      _errorCode = 2;  // checksum error
      _logger.println(F("... checksum error"));
      memset(_rcvBuffer, 0, 12);
      setState(INIT);
      return;
    }
    if (_rcvBuffer[3] == 0x01) {
      // message is from READ command, so returning read value
    }
    setState(RECEIVE_ACK);
    setAction(RETURN);
    _errorCode = 0;
    _logger.println(F("... success"));
    return;
  } else {
    // _logger.println(F("Recieved answer of unexpected length. Got:"));
    // _logger.println(_rcvBufferLen, DEC);
    // _logger.println(F("Expected:"));
    // _logger.println(_rcvLen, DEC);
  }
  if (millis() - _lastMillis > 10 * 1000UL) {  // Vitotronic isn't answering, try again
    _rcvBufferLen = 0;
    _errorCode = 1;  // Connection error
    memset(_rcvBuffer, 0, 12);
  }
}

// send Ack on message receive succes
void OptolinkP300::_receiveAckHandler() {
  const uint8_t buff[] = {0x06};
  _stream->write(buff, sizeof(buff));
  _lastMillis = millis();
  setState(INIT);
}

// set properties for datapoint and move state to SEND
bool OptolinkP300::readFromDP(uint16_t address, uint8_t length) {
  if (_action != WAIT) {
    _logger.println(F("Optolink not available, skipping action."));
    return false;
  }
  // setup properties for next state in communicationHandler
  _address = address;
  _length = length;
  _writeMessageType = false;
  _rcvBufferLen = 0;
  _numberOfTries = 5;
  memset(_rcvBuffer, 0, 12);
  setState(SEND);
  setAction(PROCESS);
  return true;
}

// set properties datapoint and move state to SEND
bool OptolinkP300::writeToDP(uint16_t address, uint8_t length, uint8_t value[]) {
  if (_action != WAIT) {
    _logger.println(F("Optolink not available, skipping action."));
    return false;
  }
  // setup variables for next state
  _address = address;
  _length = length;
  memcpy(_value, value, _length);
  _writeMessageType = true;
  _rcvBufferLen = 0;
  _numberOfTries = 5;
  memset(_rcvBuffer, 0, 12);
  setState(SEND);
  setAction(PROCESS);
  return true;
}

const int8_t OptolinkP300::available() const {
  if (_action == RETURN_ERROR)
    return -1;
  else if (_action == RETURN)
    return 1;
  else
    return 0;
}

const bool OptolinkP300::isBusy() const {
  if (_action == WAIT)
    return false;
  else
    return true;
}

// return value and reset comunication to IDLE
void OptolinkP300::read(uint8_t value[]) {
  if (_action != RETURN) {
    _logger.println(F("No reading available"));
    return;
  }
  if (_writeMessageType) {  // return original value in case of WRITE command
    memcpy(value, &_value, _length);
    _logger.print("value transferred: ");
    _printHex(&_logger, value, _length);
    _logger.println("");
    setAction(WAIT);
    return;
  } else {
    memcpy(value, &_rcvBuffer[7], _length);
    _logger.print("value transferred: ");
    _printHex(&_logger, value, _length);
    _logger.println("");
    setAction(WAIT);
    return;
  }
}

const uint8_t OptolinkP300::readError() {
  setAction(WAIT);
  _logger.println(F("Read error"));
  return _errorCode;
}

inline uint8_t OptolinkP300::_calcChecksum(uint8_t array[], uint8_t length) {
  uint8_t sum = 0;
  for (uint8_t i = 1; i < length - 1; ++i) {  // start with second byte and en before checksum
    sum += array[i];
  }
  return sum;
}
inline bool OptolinkP300::_checkChecksum(uint8_t array[], uint8_t length) {
  uint8_t sum = 0;
  for (uint8_t i = 1; i < length - 1; ++i) {  // start with second byte and en before checksum
    sum += array[i];
  }
  return (array[length - 1] == sum);
}

inline void OptolinkP300::_clearInputBuffer() {
  while (_stream->available() > 0) {
    _stream->read();
  }
}

void OptolinkP300::setLogger(Print* printer) { _logger.setPrinter(printer); }

Logger* OptolinkP300::getLogger() { return &_logger; }

// Copied from Arduino.cc forum --> (C) robtillaart
inline void OptolinkP300::_printHex(Print* printer, uint8_t array[], uint8_t length) {
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
