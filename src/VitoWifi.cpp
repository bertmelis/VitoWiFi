#include "VitoWifi.h"


//constructor
VitoWifi::VitoWifi(){
  _serial = NULL;
  Logger logger;
}


//destructor --> needs to be populated when there's need to destruct
VitoWifi::~VitoWifi(){
}


//Loop handler for ViComm: connection, sending and receiving
void VitoWifi::loop(){
  _connectionHandler();
  _communicationHandler();
}



void VitoWifi::begin(HardwareSerial* serial){
	//Start serial to Viessmann @4800/8E2
	_serial = serial;
  _serial->begin(4800, SERIAL_8E2);
  getLogger() << F("Starting ViComm communication.") << endl;
}


//Connection handler, is called in loop()
void VitoWifi::_connectionHandler(){
  switch(_connectionState){
    case RESET:
      //undefined state, keep sending 0x04 to reset
      getLogger() << F("Resetting VitoWifi connection...");
      if(_serial->available() > 0){
        //use peek so connection can be made immediately in next state
        _rcvBuffer[0] = _serial->peek();
        if(_rcvBuffer[0] == 0x05){
          _connectionState = INIT;
          _timeoutTimer = millis();
          getLogger() << F("reset.") << endl << endl;
        }
      }
      if(millis() - _lastMillis > 500){
        _sndBuffer[0] = 0x04;
        _serial->write(_sndBuffer, 1);
        getLogger() << F(".");
      }
      break;

    case INIT:
      if(_serial->available() > 0){
        _rcvBuffer[0] = _serial->read(); //use peak so connection can be made immediately
        if(_rcvBuffer[0] == 0x05){
          //0x05=INIT? received, request to connect hence sending initiator
          _sndBuffer[0] = 0x16;
          _sndBuffer[1] = 0x00;
          _sndBuffer[2] = 0x00;
          _serial->write(_sndBuffer, 3);
          _timeoutTimer = millis();
          getLogger() << F("Initiating connection...");
        }
        else if(_rcvBuffer[0] == 0x06){
          _connectionState = CONNECTED;
          _timeoutTimer = millis();
          getLogger() << F("connected...") << endl << endl;
        }
      }
      break;

    case CONNECTED:
      if(millis() - _timeoutTimer > 360000){
        //timeout of 6 minutes
        _connectionState = RESET;
        getLogger() << F("Connection has been reset: timeout") << endl;
      }
      break;
  }
}


//Communication handler, is called in loop()
void VitoWifi::_communicationHandler(){
  if(_connectionState == CONNECTED && _errorCount <= 5){
    switch(_communicationState){
      case IDLE:
        //wait for action
        getLogger() << F("Ready for new command.") << endl;
        break;

      case SEND:
        //send message
        if(!_messageSent){
          //only send a command every 100msec to avoid congestion
          if(_lastMillis - millis() > 100) _serial->write(_sndBuffer, _sndLen);
          _timeoutTimer = millis();
          _lastMillis = _timeoutTimer;
          _messageSent = true;
          getLogger() << F("Sending command for ");
          getLogger() << _DP->name;
          getLogger() << F(".\nData: ");
          _printHex83(_sndBuffer, _sndLen);
          getLogger() << endl;
        }
        //wait for acknowledgement
        if(_serial->available() > 0){
          _rcvBuffer[0] = _serial->read();
          if(_rcvBuffer[0] == 0x06){
            //transmit succesful, moving to next state
            _timeoutTimer = millis();
            _communicationState = RECEIVE;
            getLogger() << F("Command sent succesfully") << endl << endl;
          }
          else if(_rcvBuffer[0] == 0x15){
            //transmit negatively acknowledged
            _timeoutTimer = millis();
            _messageSent = false;
            _errorCount++;
            getLogger() << F("Message sent unsuccesfully, trying again") << endl << endl;
          }
        }
        break;

      case RECEIVE:
          if(_serial->available() > 0){
            _rcvBuffer[_rcvBufferLen] = _serial->read();
            _rcvBufferLen++;
            _timeoutTimer = millis();
          }
          if(_rcvBufferLen == _rcvLen){
            //message complete, check/decode
            bool flagError = false;
            if(_rcvBuffer[1] != (_rcvLen - 4)) flagError = true; //length wrong
            if(_rcvBuffer[2] != 0x01) flagError = true; //should be type "response"
            if(!_checkChecksum(_rcvBuffer, _rcvLen)) flagError = true; //check checksum
            if(flagError){
              _errorCount++;
              _messageSent = true;
              _communicationState = SEND;
              getLogger() << F("Message received unsuccesfully, trying again") << endl;
              break;
            }
            if(_rcvBuffer[3] == 0x01){
              //message is response from READ, so value is returned
              //put value in temporary buffer
              switch(_rcvLen - 8){
                case 1:
                  //value is 1 bit or 1 byte
                  _value = _rcvBuffer[8] & 0xFF;
                  break;
                case 2:
                  //value is 2 bytes
                  _value = _rcvBuffer[9] << 8 |
                           _rcvBuffer[8];
                  break;
                case 4:
                  //value is 4 bytes
                  _value = _rcvBuffer[11] << 24 |
                           _rcvBuffer[10] << 16 |
                           _rcvBuffer[9] << 8 |
                           _rcvBuffer[8];
                  break;
              }
            }
            _communicationState = RETURN;
            getLogger() << F("Message received succesfully.") << endl << endl;
          }
        break;

      case RETURN:
        //wait for action
        break;
    }
  }
  else if(_connectionState == CONNECTED && _errorCount > 5){
    _connectionState = RESET;
    getLogger() << F("Connection has been reset: too many errors") << endl << endl;
  }
}


//construct message from datapoint and move state to SEND
void VitoWifi::sendDP(Datapoint* DP){
  if(_communicationState != IDLE) getLogger() << F("Warning: sendDP before previous value was cleared.") << endl;

  _DP = DP;

  switch(_DP->RW){
    case READ:
      //construct READ message
      //has fixed length of 8 chars
      _sndLen = 8;
      _sndBuffer[0] = 0x41;
      _sndBuffer[1] = 0x05;
      _sndBuffer[2] = 0x00;
      _sndBuffer[3] = 0x01;
      _sndBuffer[4] = (_DP->address >> 8) & 0xFF;
      _sndBuffer[5] = _DP->address & 0xFF;
      _sndBuffer[6] = _DP->length;
      _sndBuffer[7] = _calcChecksum(_sndBuffer, _sndLen);

      //set lentgh of expected answer
      _rcvLen = _sndLen + _DP->length;

      //setup properties for next state in communicationHandler
      _messageSent = false;
      _communicationState = SEND;
      break;

    case WRITE:
      //should be in overloaded method
      getLogger() << F("Warning: Write request without value.") << endl;
      break;
  }
}


//overload method for write request
//construct message from datapoint and move state to SEND
void VitoWifi::sendDP(Datapoint* DP, uint32_t value){
  if(_communicationState != IDLE) getLogger() << F("Warning: sendDP before previous value was cleared.") << endl;

  _DP = DP;

  switch(_DP->RW){
    case READ:
      //should be in primary method
      getLogger() << F("Warning: Read request with value specified.") << endl;
      break;
    case WRITE:
      //construct READ message
      //has length of 8 chars + length of value
      _sndLen = 8 + _DP->length;
      _sndBuffer[0] = 0x41;
      _sndBuffer[1] = 5 + _DP->length;
      _sndBuffer[2] = 0x00;
      _sndBuffer[3] = 0x02;
      _sndBuffer[4] = (_DP->address >> 8) & 0xFF;
      _sndBuffer[5] = _DP->address & 0xFF;
      _sndBuffer[6] = _DP->length;
      //add value to message
      for(uint8_t i = 0; i < _DP->length; i++){
        _sndBuffer[6 + i] = (value >> (8 * i)) & 0xFF;
      }
      _sndBuffer[7 + _sndLen] = _calcChecksum(_sndBuffer, _sndLen);

      //set lentgh of expected answer
      _rcvLen = 8;

      //setup properties for next state in communicationHandler
      _messageSent = false;
      _communicationState = SEND;
      break;
  }
}


CommunicationState VitoWifi::getStatus(){
  return _communicationState;
}

//return value and reset comunication to IDLE
int32_t VitoWifi::getValue(){
  if(_communicationState != RETURN) getLogger() << F("Warning: getValue before it was read.") << endl;
  _communicationState = IDLE;
  return _value;
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
  uint8_t value[4] = {0};
  _value = 0;
  if(_rcvBuffer[0] != 0x06) return false;
  if(_rcvBuffer[2] != (_rcvLen - 4)) return false;
  if(_rcvBuffer[3] != 0x01) return false;
  if(!_checkChecksum(_rcvBuffer, _rcvLen)) return false;
  for(uint8_t i = 7 + _rcvBuffer[7]; i > 7; i--){
    _value = _value | _rcvBuffer[i];
    _value = _value << 8;
  }
  return true;
}
//clear serial input buffer
void VitoWifi::_clearInputBuffer(){
  //_logger->print(F("Received \"0x"));
  if(_serial->available() > 0){
    int read = _serial->read();
    uint8_t byte;
    byte |= read;
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
  getLogger() << tmp;
}
