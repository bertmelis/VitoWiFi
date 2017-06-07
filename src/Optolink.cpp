#include "Optolink.h"

/*
const char error0 [] PROGMEM = "succes";
const char error1 [] PROGMEM = "error1";
const char error2 [] PROGMEM = "error2";
const char error3 [] PROGMEM = "error3";
const char error4 [] PROGMEM = "error4";
const char error5 [] PROGMEM = "error5";

const char* const errorMessages[] PROGMEM = {
  error0,
  error1,
  error2,
  error3,
  error4,
  error5
};
*/

Optolink::Optolink():
_serial(nullptr),
_address(0),
_length(0),
_value{0},
_writeMessageType(false),
_rcvBuffer{0},
_rcvBufferLen(0),
_rcvLen(0),
_timeoutTimer(0),
_lastMillis(0),
_errorCount(0),
_sendMessage(false),
_debugMessage(true),
_state(RESET),
_errorCode(0),
_debugPrinter(nullptr)
{}


//begin serial @ 4800 baud, 8 bits, even parity, 2 stop bits
void Optolink::begin(HardwareSerial* serial) {
  _serial = serial;
  _serial->begin(4800, SERIAL_8E2);
}


void Optolink::loop() {
  if (millis() - _timeoutTimer > 6 * 60 * 1000UL) {
    //timeout in 6 minutes, reset
    _state = RESET;
    _debugPrinter->print(F("connection reset: timeout"));
    return;
  }
  if (_errorCount >= 5 ) {
    //5 errors, reset
    _state = RESET;
    _errorCount = 0;
    _debugPrinter->print(F("connection reset: max errors exceeded."));
    return;
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
    case SYNC:
      _syncHandler();
      break;
    case SYNC_ACK:
      _syncAckHandler();
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
    case RETURN:
      _returnHandler();
      break;
  }
}


//Set communication with Vitotronic to defined state = reset to KW protocol
void Optolink::_resetHandler() {
  const uint8_t buff[] = {0x04};
  _serial->write(buff, sizeof(buff));
  _lastMillis = millis();
  _state = RESET_ACK;
  if (_debugMessage) {
    _debugPrinter->println(F("Resetting Optolink..."));
    _debugMessage = false;
  }
  return;
}
void Optolink::_resetAckHandler() {
  if (_serial->available()) {
    if (_serial->peek() == 0x05) { //use peek so connection can be made immediately in next state
      //received 0x05/enquiry: optolink has been reset
      _state = INIT;
      _timeoutTimer = millis();
      _debugMessage = true;
      _debugPrinter->println(F("Optolink reset."));
    }
    else {
      _clearInputBuffer();
    }
  }
  else {
    if (millis() - _lastMillis > 500) {
      _state = RESET;
    }
  }
}


//send initiator to Vitotronic to establish connection
void Optolink::_initHandler() {
  if (_debugMessage) {
    _debugPrinter->println(F("Establishing Optolink connection..."));
    _debugMessage = false;
  }
  if (_serial->available()) {
    if (_serial->read() == 0x05) {
      //0x05/ENQ received, sending initiator
      const uint8_t buff[] = {0x16, 0x00, 0x00};
      _serial->write(buff, sizeof(buff));
      _lastMillis = millis();
      _timeoutTimer = _lastMillis;
      _state = INIT_ACK;
    }
  }
  return;
}
void Optolink::_initAckHandler() {
  if (_serial->available()) {
    if (_serial->read() == 0x06) {
      //ACK received, moving to next state
      _state = IDLE;
      _timeoutTimer = millis();
      _debugMessage = true;
      _errorCount = 0;
      //debug: done
      _debugPrinter->println(F("Optolink connection established."));
    }
    else {
      //return to previous state
      _clearInputBuffer();
      _state = INIT;
    }
  }
  return;
}


//idle state, waiting for user action
void Optolink::_idleHandler() {
  if (millis() - _lastMillis > 2 * 60 * 1000UL) {
    //send SYNC every 2 minutes to keep communication alive
    _state = SYNC;
  }
  _clearInputBuffer(); //keep input clean
  if (_sendMessage) _state = SYNC;
  else _state = IDLE;
  return;
}


//send SYNC (= initiator)
void Optolink::_syncHandler() {
  if (_debugMessage) {
    _debugPrinter->println(F("Syncing Optolink..."));
    _debugMessage = false;
  }
  const uint8_t buff[] = {0x16, 0x00, 0x00};
  _serial->write(buff, sizeof(buff));
  _state = SYNC_ACK;
  _lastMillis = millis();
  return;
}


void Optolink::_syncAckHandler() {
  if (_serial->available()) {
    if (_serial->read() == 0x06) {
      _timeoutTimer = millis();
      _debugMessage = true;
      _debugPrinter->println(F("Optolink synced."));
      if(_sendMessage) _state = SEND;
      else _state = IDLE;
    }
    else {
      _errorCount++;
    }
  }
  return;
}


void Optolink::_sendHandler() {
  uint8_t buff[12];
  if (_writeMessageType) {
    //type is WRITE
    //has length of 8 chars + length of value
    buff[0] = 0x41;
    buff[1] = 5 + _length;
    buff[2] = 0x00;
    buff[3] = 0x02;
    buff[4] = (_address >> 8) & 0xFF;
    buff[5] = _address & 0xFF;
    buff[6] = _length;
    //add value to message
    memcpy(&buff[7], _value, 1);
    buff[7 + _length] = _calcChecksum(buff, 7 + _length);
    _serial->write(buff, 8 + _length);
  }
  else {
    //type is READ
    //has fixed length of 8 chars
    buff[0] = 0x41;
    buff[1] = 0x05;
    buff[2] = 0x00;
    buff[3] = 0x01;
    buff[4] = (_address >> 8) & 0xFF;
    buff[5] = _address & 0xFF;
    buff[6] = _length;
    buff[7] = _calcChecksum(buff, 8);
    //set length of expected answer
    _rcvLen = 8 + _length;
    _serial->write(buff, 8);
  }
  _lastMillis = millis();
  _sendMessage = false;
  _rcvBufferLen = 0;
  _state = SEND_ACK;
  _debugPrinter->print(F("Message sent for "));
  if (_writeMessageType) _debugPrinter->print(F("WRITE "));
  else _debugPrinter->print(F("READ "));
  _debugPrinter->print(F("on address "));
  _debugPrinter->print(_address, HEX);
  _debugPrinter->print(F(" with length "));
  _debugPrinter->print(_length);
  _debugPrinter->println(F("."));
  return;
}


void Optolink::_sendAckHandler() {
  if (_serial->available()) {
    uint8_t buff = _serial->read();
    _timeoutTimer = millis();
    if (buff == 0x06) {
      //transmit succesful, moving to next state
      _state = RECEIVE;
      _debugPrinter->println(F("Message send ACK."));
    }
    else if (buff == 0x15) {
      //transmit negatively acknowledged, return to SYNC and try again
      _timeoutTimer = millis();
      _sendMessage = true;
      _errorCount++;
      _state = SYNC;
      _clearInputBuffer();
      _debugPrinter->println(F("Message send NACK, trying again"));
    }
  }
  else if (millis() - _timeoutTimer > 2 * 1000UL) {
    //timeout waiting for ACK 2secs, return to SYNC and try again
    _sendMessage = true;
    _state = SYNC;
    _errorCount++;
    _clearInputBuffer();
    _debugPrinter->println(F("Message send timeout, trying again"));
  }
  return;
}


void Optolink::_receiveHandler() {
  while (_serial->available() > 0) {  //while instead of if: read complete RX buffer
    _rcvBuffer[_rcvBufferLen] = _serial->read();
    if (_rcvBuffer[0] != 0x41) return; //find out why this is needed! I'd expect the rx-buffer to be empty.
    _rcvBufferLen++;
    _timeoutTimer = millis();
  }
  if (_rcvBufferLen == _rcvLen) {
    //message complete, check/decode
    _debugPrinter->println(F("Message received, checking."));
    if (_rcvBuffer[1] != (_rcvLen - 3)) {
      _errorCount++;
      _sendMessage = false;
      _state = RETURN;
      _errorCode = 2;
      _debugPrinter->println(F("Length check error, skipping command"));
      return;
    }
    if (!_checkChecksum(_rcvBuffer, _rcvLen)) {
      //checksum is wrong, trying again
      _errorCount++;
      _sendMessage = true;
      _rcvBufferLen = 0;
      memset(_rcvBuffer, 0, 12);
      _state = SYNC;
      _debugPrinter->println(F("Checksum error, trying again"));
      return;
    }
    if (_rcvBuffer[2] != 0x01) {
      //Vitotronic returns a failure message, skipping DP
      _errorCount++;
      _sendMessage = false;
      _state = RETURN;
      _errorCode = 3;
      _debugPrinter->println(F("command NOK, skipping command"));
      return;
    }
    if (_rcvBuffer[3] == 0x01) {
      //message is response from READ, so value is returned correctly
      //ESP8266 is little endian so least significant byte first
      //debug: Succes
      //old solution: _value.byte4Value = (_rcvBuffer[10] << 24) | (_rcvBuffer[9] << 16) | (_rcvBuffer[8] << 8) | _rcvBuffer[7];
      }
      _state = RETURN;
    }
  return;
}


void Optolink::_returnHandler() {
  //stay in this state untill message has been read
  //waiting for read()-action
}


//set properties for datapoint and move state to SEND
bool Optolink::readFromDP(uint16_t address, uint8_t length) {
  if (_state != IDLE && (_state == SYNC && _sendMessage)) {
    _debugPrinter->println(F("Previous process not finished, skipping this action"));
    return false;
  }
  //setup properties for next state in communicationHandler
  _address = address;
  _length = length;
  _writeMessageType = false;
  _sendMessage = true;
  _rcvBufferLen = 0;
  memset(_rcvBuffer, 0, 12);
  _state = SYNC;
  return true;
}


//set properties datapoint and move state to SEND
bool Optolink::writeToDP(uint16_t address, uint8_t length, uint8_t value[]) {
  if (_state != IDLE && (_state == SYNC && _sendMessage)) {
    _debugPrinter->println(F("Previous process not finished, skipping this action"));
    return false;
  }
  //setup variables for next state
  _address = address;
  _length = length;
  memcpy(_value, value, _length);
  _writeMessageType = true;
  _sendMessage = true;
  _rcvBufferLen = 0;
  memset(_rcvBuffer, 0, 12);
  _state = SYNC;
  return true;
}


const int8_t Optolink::available() {
  if (_errorCode) return -1;
  else if (_state == RETURN) return 1;
  else return 0;
}


//return value and reset comunication to IDLE
void Optolink::read(uint8_t value[]) {
  if (_state != RETURN) {
    _debugPrinter->println(F("Optolink warning, read value before it is available."));
  }
  if (_writeMessageType) {  //return original value in case of WRITE command
    memcpy(value, &_value, _length);
    _state = IDLE;
    return;
  }
  else {
    memcpy(value, &_rcvBuffer[7], _length);
    _state = IDLE;
    return;
  }
}


//calculate Checksum
inline uint8_t Optolink::_calcChecksum(uint8_t array[], uint8_t length) {
  uint8_t sum = 0;
  for (uint8_t i = 1; i < length - 1 ; i++){  //start with second byte and en before checksum
    sum += array[i];
  }
  return sum;
}
inline bool Optolink::_checkChecksum(uint8_t array[], uint8_t length) {
  uint8_t sum = 0;
  for (uint8_t i = 1; i < length - 1; i++) { //start with second byte and en before checksum
    sum += array[i];
  }
  return (array[length - 1] == sum);
}


//clear serial input buffer
inline void Optolink::_clearInputBuffer() {
  while(_serial->available() > 0) {
    _serial->read();
  }
}

void Optolink::setDebugPrinter(Print* printer) {
  _debugPrinter = printer;
}

void Optolink::readError(char* errorString, uint8_t length) const  {
  //copy errorMessage to errorString, keeping an eye on length
  strncpy(errorString, "this is an errorMessage", length - 1);
  return;
};

//Copied from Arduino.cc forum --> (C) robtillaart
inline void Optolink::_printHex83(uint8_t array[], uint8_t length) {
  char tmp[length * 2 + 1];
  byte first;
  uint8_t j = 0;
  for (uint8_t i = 0; i < length; i++) {
    first = (array[i] >> 4) | 48;
    if (first > 57) tmp[j] = first + (byte)39;
    else tmp[j] = first ;
    j++;

    first = (array[i] & 0x0F) | 48;
    if (first > 57) tmp[j] = first + (byte)39;
    else tmp[j] = first;
    j++;
  }
  tmp[length * 2] = 0;
  _debugPrinter->print(tmp);
}
