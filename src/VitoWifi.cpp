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
  connectionHandler();
  communicationHandler();
}


void VitoWifi::begin(HardwareSerial* serial){
	//Start serial to Viessmann @4800/8E2
	_serial = serial;
  _serial->begin(4800, SERIAL_8E2);
  _timeoutTimer = 0;
  _lastMillis = 0;
  _debugMessage = true;
  _connectionState = RESET;
  _sendMessage = false;
  _communicationState = IDLE;
  getLogger().println(F("VitoWifi: Setup done."));
}


//Connection handler, is called in loop()
void VitoWifi::connectionHandler(){
  switch(_connectionState){
    case RESET: //reset connection: keep sending 0x04/EOT to reset optolink
      if(_debugMessage){
        getLogger().print(F("VitoWifi: Resetting optolink communication..."));
        _debugMessage = false;
      }
      if(_serial->available()){
        //use peek so connection can be made immediately in next state
        _rcvBuffer[0] = _serial->peek();
        if(_rcvBuffer[0] == 0x05){
          //received 0x05/enquiry: optolink has been reset
          _connectionState = INIT;
          _timeoutTimer = millis();
          _debugMessage = true;
          getLogger().println(F("reset.\n"));
        }
      }
      if(millis() - _lastMillis > 500){
        //send reset request 0x04/EOT every 500msec
        _sndBuffer[0] = 0x04;
        _serial->write(_sndBuffer, 1);
        _lastMillis = millis();
        getLogger().print(F("."));
      }
      break;

    case INIT:
      if(_debugMessage){
        getLogger().print(F("VitoWifi: Initializing connection..."));
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
          getLogger().print(F("."));
        }
        if(_rcvBuffer[0] == 0x06){
          _connectionState = CONNECTED;
          _communicationState = IDLE;
          _timeoutTimer = millis();
          _debugMessage = true;
          clearInputBuffer();
          getLogger().println(F("connected.\n"));
        }
      }
      break;

    case CONNECTED:
      if(millis() - _timeoutTimer > 360000){
        //timeout of 6 minutes
        _connectionState = RESET;
        getLogger().println(F("VitoWifi: Connection has been reset: timeout\n"));
      }
      break;
  }
}


//Communication handler, is called in loop()
void VitoWifi::communicationHandler(){
  if(_connectionState != CONNECTED) return;
  if(_errorCount >= 5){
    _connectionState = RESET;
    getLogger().println(F("VitoWifi: Connection has been reset: too many errors.\n"));
    return;
  }
  switch(_communicationState){
    case IDLE:
      //stay in this state untill sendDP has been called.
      //display message only once.
      if(_debugMessage){
        getLogger().println(F("VitoWifi: Ready for new command."));
        _debugMessage = false;
      }
      clearInputBuffer();
      break;

    case SEND:
      //stay in this state untill message has been acknowledged
      //but send message once or after feedback from vitotronic
      if(_sendMessage){
        //only send a command every 100msec to avoid congestion
        //can be removed after testing/debugging
        if(_lastMillis - millis() > 100) _serial->write(_sndBuffer, _sndLen);
        _timeoutTimer = millis();
        _lastMillis = _timeoutTimer;
        _sendMessage = false;
        getLogger().print(F("VitoWifi: Sending command for "));
        getLogger().println(_DP->name);;
        getLogger().print(F("          Data: "));
        printHex83(_sndBuffer, _sndLen);
        getLogger().println();
      }
      //wait for acknowledgement
      if(_serial->available()){
        _rcvBuffer[0] = _serial->read();
        if(_rcvBuffer[0] == 0x06){
          //transmit succesful, moving to next state
          _timeoutTimer = millis();
          _communicationState = RECEIVE;
          getLogger().println(F("VitoWifi: Command sent succesfully."));
        }
        if(_rcvBuffer[0] == 0x15){
          //transmit negatively acknowledged
          _timeoutTimer = millis();
          _sendMessage = true;
          _errorCount++;
          clearInputBuffer();
          getLogger().println(F("VitoWifi: Command sent unsuccesfully, trying again"));
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
        if(!checkChecksum(_rcvBuffer, _rcvLen)) flagError = true; //check checksum
        if(flagError){
          _errorCount++;
          _sendMessage = false;
          _communicationState = SEND;
          getLogger().println(F("Message received unsuccesfully, trying again"));
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
          getLogger().println(F("Message received succesfully.\n"));
        }
      break;

    case RETURN:
      //stay in this state untill value has been read. (getValue)
      break;
    }
}


//construct message from datapoint and move state to SEND
void VitoWifi::sendDP(Datapoint* DP){
  if(_communicationState != IDLE) getLogger().println(F("Warning: sendDP before previous action was completed. Using new DP."));
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
      _sndBuffer[7] = calcChecksum(_sndBuffer, _sndLen);

      //set length of expected answer
      _rcvLen = _sndLen + _DP->length;

      //setup properties for next state in communicationHandler
      _sendMessage = true;
      _communicationState = SEND;
      break;

    case WRITE:
      //should be in overloaded method
      getLogger().println(F("VitoWifi: Error! Write request without value argument. Skipping!"));
      _sendMessage = false;
      _value = 0;
      _communicationState = RETURN;
      break;
  }
}


//overload method for write request
//construct message from datapoint and move state to SEND
void VitoWifi::sendDP(Datapoint* DP, uint32_t value){
  if(_communicationState != IDLE) getLogger().println(F("Warning: sendDP before previous value was cleared."));
  _DP = DP;
  switch(_DP->RW){
    case READ:
      //should be in primary method
      getLogger().println(F("VitoWifi: Error! Read request with value argument. Skipping!"));
      _sendMessage = false;
      _value = 0;
      _communicationState = RETURN;
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
      _sndBuffer[7 + _sndLen] = calcChecksum(_sndBuffer, _sndLen);

      //set lentgh of expected answer
      _rcvLen = 8;

      //setup properties for next state in communicationHandler
      _sendMessage = true;
      _communicationState = SEND;
      break;
  }
}


CommunicationState VitoWifi::getStatus(){
  return _communicationState;
}

//return value and reset comunication to IDLE
float VitoWifi::getValue(){
  if(_communicationState != RETURN) getLogger().println(F("Warning: getValue before it was read."));
  _communicationState = IDLE;
  return transform(_value);
}


float VitoWifi::transform(int32_t value){
  switch(_DP->transformation){
    case TEMP:    //temperature
      return (value / 10.0);
    case H:       //hours
      break;
    case C:      //counter
      break;
    case E_OM1:  //enum operating mode (stanby/only dhw/heating+dhw/red/std)
      break;
    case E_OM2:  //enum operating mode heating (standby/reduced/norm1/norm2)
      break;
    case E_S:    //enum status
      break;
    case E_SF:   //enum status flame (B2)
      break;
    case E_M:    //enum mode (B1)
      break;
    case E_PM:   //enum party mode (B7)
      break;
    case E_FW:   //enum frost warning (B7)
      break;
    case TXT:    //text
      break;
    case E_SV:   //switch valve (undef/heating/middle/dhw)
      break;
    case NONE:
    default:
      break;
  }
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
uint8_t VitoWifi::calcChecksum(uint8_t* message, uint8_t lenght){
  uint8_t sum = 0;
  for(uint8_t i = 1; i < lenght - 1; i++){
    sum = sum + message[i];
  }
  return (sum & 0xFF);
}
bool VitoWifi::checkChecksum(uint8_t* message, uint8_t length){
  uint8_t sum = 0;
  for(uint8_t i = 1; i < length - 1; i++){
    sum = sum + message[i];
  }
  sum = sum & 0xFF;
  return (message[length - 1] == sum);
}
bool VitoWifi::decodeMessage(){
  uint8_t value[4] = {0};
  _value = 0;
  if(_rcvBuffer[0] != 0x06) return false;
  if(_rcvBuffer[2] != (_rcvLen - 4)) return false;
  if(_rcvBuffer[3] != 0x01) return false;
  if(!checkChecksum(_rcvBuffer, _rcvLen)) return false;
  for(uint8_t i = 7 + _rcvBuffer[7]; i > 7; i--){
    _value = _value | _rcvBuffer[i];
    _value = _value << 8;
  }
  return true;
}
//clear serial input buffer
void VitoWifi::clearInputBuffer(){
  //_logger->print(F("Received \"0x"));
  uint8_t byte = 0;
  if(_serial->available() > 0){
    byte |= _serial->read();
    //_printHex83(&byte, 1);
  }
  //_logger->println(F("\", but nothing expecting. Discarted."));
}
//Copied from Arduino.cc forum --> (C) robtillaart
void VitoWifi::printHex83(uint8_t *data, uint8_t length){
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
