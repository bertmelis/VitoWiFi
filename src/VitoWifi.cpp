#include "VitoWifi.h"


//constructor
VitoWifi::VitoWifi():
  _serial(NULL),
  _sndBuffer{0},
  _sndLen(0),
  _rcvBuffer{0},
  _rcvBufferLen(0),
  _rcvLen(0),
  _timeoutTimer(0),
  _lastMillis(0),
  _errorCount(0),
  _sendMessage(false),
  _sendSync(true),
  _debugMessage(true),
  _state(RESET)
  //_returnValue.byte4Value = 0 as initialized in the union
  {}

//destructor --> needs to be implemented when there's need to destruct
VitoWifi::~VitoWifi(){
}


//Loop handler, state machine
void VitoWifi::loop(){
  switch(_state){
    case RESET:
      _resetHandler();
      break;
    case INIT:
      _initHandler();
      break;
    case IDLE:
      _idleHandler();
      break;
    case SYNC:
      _syncHandler();
      break;
    case SEND:
      _sendHandler();
      break;
    case RECEIVE:
      _receiveHandler();
      break;
    case RETURN:
      _returnHandler();
      break;
  }
}


//Start serial to Viessmann @4800/8E2
void VitoWifi::begin(HardwareSerial& serial){
	begin(serial);
}
void VitoWifi::begin(HardwareSerial* serial){
	_serial = serial;
	_serial->begin(4800, SERIAL_8E2);
	getLogger().println(F("VitoWifi: Setup...done"));
}


//Set communication with Vitotronic to defined state = reset to KW protocol
void VitoWifi::_resetHandler(){
  if(_debugMessage){
    getLogger().print(F("VitoWifi: Resetting..."));
    _debugMessage = false;
  }
  if(_serial->available()){
    //use peek so connection can be made immediately in next state
    _rcvBuffer[0] = _serial->peek();
    if(_rcvBuffer[0] == 0x05){
      //received 0x05/enquiry: optolink has been reset
      _state = INIT;
      _timeoutTimer = millis();
      _debugMessage = true;
      _rcvBuffer[0] = 0;
      getLogger().println(F("done"));
    }
    else
      _clearInputBuffer();
  }
  if(millis() - _lastMillis > 500){
    //send reset request 0x04/EOT every 500msec
    _sndBuffer[0] = 0x04;
    _serial->write(_sndBuffer, 1);
    _lastMillis = millis();
  }
  return;
}


void VitoWifi::_initHandler(){
  if(_debugMessage){
    getLogger().print(F("VitoWifi: Syncing..."));
    _debugMessage = false;
  }
  if(_serial->available()){
    _rcvBuffer[0] = _serial->read();
    if(_rcvBuffer[0] == 0x05){
      //0x05/enquiry received, request to connect hence sending initiator
      _sndBuffer[0] = 0x16;
      _sndBuffer[1] = 0x00;
      _sndBuffer[2] = 0x00;
      _serial->write(_sndBuffer, 3);
      _timeoutTimer = millis();
    }
    if(_rcvBuffer[0] == 0x06){
      //ACK received, moving to next state
      _state = IDLE;
      _timeoutTimer = millis();
      _debugMessage = true;
      _errorCount = 0;
      getLogger().println(F("done"));
      getLogger().println(F("          Connection established"));
    }
  }
  return;
}


void VitoWifi::_idleHandler(){
  if(millis() - _lastMillis > 2* 60 * 1000UL){
    //send SYNC every 2 minutes to keep communication alive
    _state = SYNC;
    _sendSync = true;
  }
  if(millis() - _timeoutTimer > 6 * 60 * 1000UL){
    //timeout in 6 minutes, reset
    _state = RESET;
    getLogger().println(F("VitoWifi: Connection reset: timeout"));
  }
  if(_errorCount >= 5){
    //5 errors, reset
    _state = RESET;
    getLogger().println(F("VitoWifi: Connection reset: too many errors."));
    return;
  }
  _clearInputBuffer(); //keep input clean
}

void VitoWifi::_syncHandler(){
  if(_debugMessage){
    getLogger().print(F("VitoWifi: Syncing..."));
    _debugMessage = false;
  }
  if(_sendSync){
    uint8_t buff[3] = {0x16, 0x00, 0x00};
    _serial->write(buff, 3);
    _sendSync = false;
    _lastMillis = millis();
  }
  if(_serial->available()) _rcvBuffer[0] = _serial->read();
  if(_rcvBuffer[0] == 0x06){
    _timeoutTimer = millis();
    _debugMessage = true;
    _errorCount = 0;
    _rcvBuffer[0] = 0;
    getLogger().println(F("done"));
    if(_sendMessage) _state = SEND;
    else _state = IDLE;
  }
  else{
    _errorCount++;
    _sendSync = true;
  }
  return;
}


void VitoWifi::_sendHandler(){
  //stay in this state untill message has been acknowledged
  //but send message only once every try
  //message payload is set in sendDP/writeDP method.
  if(_sendMessage){
    _serial->write(_sndBuffer, _sndLen);
    _timeoutTimer = millis();
    _lastMillis = _timeoutTimer;
    _sendMessage = false;
    _rcvBufferLen = 0;
    getLogger().print(F("VitoWifi: Sending command for "));
    getLogger().println(_DP.name);
    getLogger().print(F("          Data: 0x"));
    _printHex83(_sndBuffer, _sndLen);
    getLogger().println();
  }
  if(_serial->available()){
    _rcvBuffer[0] = _serial->read();
    if(_rcvBuffer[0] == 0x06){
      //transmit succesful, moving to next state
      _timeoutTimer = millis();
      _state = RECEIVE;
      _rcvBufferLen = 0;
      _clearInputBuffer();
      getLogger().println(F("          Succes"));
    }
    if(_rcvBuffer[0] == 0x15){
      //transmit negatively acknowledged
      _timeoutTimer = millis();
      _sendMessage = true;
      _errorCount++;
      _rcvBuffer[0] = 0;
      _clearInputBuffer();
      getLogger().println(F("          CRC error, trying again"));
    }
  }
  if(millis() - _timeoutTimer > 1000){
    //timeout waiting for ACK 1sec
    _sendMessage = true;
    _errorCount++;
    _clearInputBuffer();
    getLogger().println(F("          Timeout error, trying again"));
  }
  return;
}


void VitoWifi::_receiveHandler(){
  static bool flagError = false;
  if(_serial->available() > 0){
    _rcvBuffer[_rcvBufferLen] = _serial->read();
    if(_rcvBuffer[0] != 0x41) return;
    _rcvBufferLen++;
    _timeoutTimer = millis();
  }
  if(_rcvBufferLen == _rcvLen){
    //message complete, check/decode
    getLogger().println(F("VitoWifi: Message received, checking"));
    getLogger().print(F("          Data: 0x"));
    _printHex83(_rcvBuffer, _rcvBufferLen);
    getLogger().println();
    if(_rcvBuffer[1] != (_rcvLen - 3)){
      _errorCount++;
      _sendMessage = false;
      _state = RETURN;
      _returnValue.byte4Value = 0;
      getLogger().println(F("          Length check error: skipping DP"));
      return;
    }
    if(!_checkChecksum(_rcvBuffer, _rcvLen)){
      //checksum is wrong, trying again
      _errorCount++;
      _sendMessage = true;
      _state = SYNC;
      getLogger().println(F("          CRC check error: trying again"));
      return;
    }
    if(_rcvBuffer[2] != 0x01){
      //Vitotronic returns a failure message, skipping DP
      _errorCount++;
      _sendMessage = false;
      _state = RETURN;
      _returnValue.byte4Value = 0;
      getLogger().println(F("          DP error: skipping DP"));
      return;
    }
    if(_rcvBuffer[3] == 0x01){
      //message is response from READ, so value is returned correctly
      //ESP8266 is little endian so least significant byte first
      getLogger().println(F("          Succes"));
      switch(_rcvLen - 8){
        case 1:
          //value is 1 bit or 1 byte
          _returnValue.byte1Value = (_rcvBuffer[7]) ? 1 : 0;
          break;
        case 2:
          //value is 2 bytes
          _returnValue.byte2Value = _rcvBuffer[8];
          _returnValue.byte2Value = _returnValue.byte2Value << 8 | _rcvBuffer[9];
          break;
        case 4:
          //value is 4 bytes
          _returnValue.byte4Value = _rcvBuffer[8];
          _returnValue.byte4Value = _returnValue.byte4Value << 8 | _rcvBuffer[9];
          _returnValue.byte4Value = _returnValue.byte4Value << 8 | _rcvBuffer[10];
          _returnValue.byte4Value = _returnValue.byte4Value << 8 | _rcvBuffer[11];
          break;
      }
      _state = RETURN;
    }
  }
  return;
}


void VitoWifi::_returnHandler(){
  //stay in this state untill message has been read
  //waiting for user action --> read()
}


//construct message from datapoint and move state to SEND
void VitoWifi::sendDP(const Datapoint& DP){
  if(_state != IDLE && (_state == SYNC && _sendMessage))
    getLogger().println(F("VitoWifi: Warning, new request before old was processed. Using new."));
  _DP = DP;
  switch(_DP.rw){
    case READ:
      //construct READ message
      //has fixed length of 8 chars
      _sndLen = 8;
      _sndBuffer[0] = 0x41;
      _sndBuffer[1] = 0x05;
      _sndBuffer[2] = 0x00;
      _sndBuffer[3] = 0x01;
      _sndBuffer[4] = (_DP.address >> 8) & 0xFF;
      _sndBuffer[5] = _DP.address & 0xFF;
      _sndBuffer[6] = _DP.length;
      _sndBuffer[7] = _calcChecksum(_sndBuffer, _sndLen);

      //set length of expected answer
      _rcvLen = _sndLen + _DP.length;

      //setup properties for next state in communicationHandler
      _sendSync = true;
      _sendMessage = true;
      _state = SYNC;
      break;

    case WRITE:
      //should be in overloaded method
      getLogger().println(F("VitoWifi: Error! Write request without value argument. Skipping."));
      _returnValue.byte4Value = 0;
      _state = RETURN;
      break;
  }
}


//overload method for write request
//construct message from datapoint and move state to SEND
void VitoWifi::sendDP(const Datapoint& DP, uint32_t value){
  if(_state != IDLE)
    getLogger().println(F("Warning: sendDP before previous value was cleared."));
  _DP = DP;
  switch(_DP.rw){
    case READ:
      //should be in primary method
      getLogger().println(F("VitoWifi: Error! Read request with value argument. Skipping!"));
      _returnValue.byte4Value = 0;
      _state = RETURN;
      break;
    case WRITE:
      //construct READ message
      //has length of 8 chars + length of value
      _sndLen = 8 + _DP.length;
      _sndBuffer[0] = 0x41;
      _sndBuffer[1] = 5 + _DP.length;
      _sndBuffer[2] = 0x00;
      _sndBuffer[3] = 0x02;
      _sndBuffer[4] = (_DP.address >> 8) & 0xFF;
      _sndBuffer[5] = _DP.address & 0xFF;
      _sndBuffer[6] = _DP.length;
      //add value to message
      for(uint8_t i = 0; i < _DP.length; i++){
        _sndBuffer[6 + i] = (value >> (8 * i)) & 0xFF;
      }
      _sndBuffer[7 + _sndLen] = _calcChecksum(_sndBuffer, _sndLen);
      //setup variables for next state
      _rcvLen = 8;
      _sendSync = true;
      _sendMessage = true;
      _state = SYNC;
      break;
  }
}


bool VitoWifi::available() const {
  return (_state == RETURN) ? true : false;
}


//return value and reset comunication to IDLE
float VitoWifi::read(){
  if(_state != RETURN)
    getLogger().println(F("VitoWifi: Warning, read before processing. Undefined output."));
  _state = IDLE;
  switch(_DP.type){
    case TEMP:    //temperature
      return (float)(_returnValue.byte2Value / 10.0);
    case H:       //hours
      return (float)_returnValue.byte4Value / 3600.0;
      break;
    case C:      //counter
      return (float)_returnValue.byte4Value;
      break;
    case E_OM1:  //enum operating mode (stanby/only dhw/heating+dhw/red/std)
      return (float)_returnValue.byte1Value;
      break;
    case E_OM2:  //enum operating mode heating (standby/reduced/norm1/norm2)
      return (float)_returnValue.byte1Value;
      break;
    case E_S:    //enum status
      return (float)(bool)_returnValue.byte1Value;
      break;
    case E_SF:   //enum status flame (B2)
      return (float)(bool)_returnValue.byte1Value;
      break;
    case E_M:    //enum mode (B1)
      return (float)_returnValue.byte1Value;
      break;
    case E_PM:   //enum party mode (B7)
      return (float)(bool)_returnValue.byte1Value;
      break;
    case E_FW:   //enum frost warning (B7)
      return (float)(bool)_returnValue.byte1Value;
      break;
    case TXT:    //text
      return 0.0; //not implemented yet
      break;
    case E_SV:   //switch valve (undef/heating/middle/dhw)
      return (float)_returnValue.byte1Value;
      break;
    case NONE:
    default:
      _state = IDLE;
      return 0.0;
      break;
  }
};
float VitoWifi::read(char* buffer, uint8_t max_buffer_size){
  if(max_buffer_size < 8){
    getLogger().println(F("VitoWifi: output buffer size too small: exiting"));
    abort();
  }
  float readValue = read();
  dtostrf(readValue, 3, 1, buffer); //copy float with minimum width of 3 and 1 number after decimal to buffer
  return readValue;
}


//Logger stuff, taken from Marvin ROGER's Homie for ESP8266
void VitoWifi::setLoggingPrinter(Print* printer){
  _logger.setPrinter(printer);
}
Logger& VitoWifi::getLogger(){
  return _logger;
}
void VitoWifi::enableLogger(bool enable){
  _logger.setLogging(enable);
}


//******************************************************************************
//helper functions

//calculate Checksum
uint8_t VitoWifi::_calcChecksum(uint8_t* message, uint8_t lenght){
  uint8_t sum = 0;
  for(uint8_t i = 1; i < lenght - 1; i++){
    sum = sum + message[i];
  }
  return (sum & 0xFF);
}
bool VitoWifi::_checkChecksum(uint8_t* message, uint8_t length){
  uint8_t sum = 0;
  for(uint8_t i = 1; i < length - 1; i++){
    sum = sum + message[i];
  }
  sum = sum & 0xFF;
  return (message[length - 1] == sum);
}
bool VitoWifi::_decodeMessage(){
  int32_t value = 0;
  if(_rcvBuffer[0] != 0x06) return false;
  if(_rcvBuffer[2] != (_rcvLen - 4)) return false;
  if(_rcvBuffer[3] != 0x01) return false;
  if(!_checkChecksum(_rcvBuffer, _rcvLen)) return false;
  for(uint8_t i = 7 + _rcvBuffer[7]; i > 7; i--){
    value = value | _rcvBuffer[i];
    value = value << 8;
  }
  return true;
}
//clear serial input buffer
void VitoWifi::_clearInputBuffer(){
  //_logger->print(F("Received \"0x"));
  uint8_t byte = 0;
  if(_serial->available() > 0){
    byte |= _serial->read();
    //_printHex83(&byte, 1);
  }
  //_logger->println(F("\", but nothing expecting. Discarted."));
}
//Copied from Arduino.cc forum --> (C) robtillaart
void VitoWifi::_printHex83(uint8_t *data, uint8_t length){
  char tmp[length * 2 + 1];
  byte first;
  int j = 0;
  for (uint8_t i = 0; i < length; i++){
    first = (data[i] >> 4) | 48;
    if (first > 57) tmp[j] = first + (byte)39;
    else tmp[j] = first ;
    j++;

    first = (data[i] & 0x0F) | 48;
    if (first > 57) tmp[j] = first + (byte)39;
    else tmp[j] = first;
    j++;
  }
  tmp[length * 2] = 0;
  getLogger().print(tmp);
}
