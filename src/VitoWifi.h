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



VitoWifi - Library serial communication with Viessmann heating systems
using the P300 protocol.
for Arduino - ESP8266

Created by: Bert Melis, 2017
Licence: MIT

Using portions or complete code from:
Hex print: 2011, robtillaart @ Arduino.cc forum
Logger: MIT 2015, marvinroger @ Github

BIG thanks to openv.wikispaces.com
and many others

*/

#pragma once

#include <Arduino.h>
#include "Logger.h"
#include "Helpers.h"

class VitoWifi{
  public:
    //setup and general methods
	  VitoWifi();
	  ~VitoWifi();
    void begin(HardwareSerial& serial);
	  void begin(HardwareSerial* serial);
    void loop();

    //communication methods
    void sendDP(const Datapoint& DP); //to read a value
    void sendDP(const Datapoint& DP, uint32_t value); //to write a value
    bool available() const; //check is action is completed eg. an answer is available
    float read(); //read the answer and return as float
    float read(char* buffer, uint8_t max_buffer_size = 8); //read the answer into your buffer

    //debugging methods
    void setLoggingPrinter(Print* printer);
    Logger& getLogger();
    void enableLogger(bool enable);

  private:
    Logger _logger;
    HardwareSerial* _serial;
    uint8_t _sndBuffer[12];
    uint8_t _sndLen;
    uint8_t _rcvBuffer[12];
    uint8_t _rcvBufferLen;
    uint8_t _rcvLen;
    uint8_t _valBuffer[4];
    Datapoint _DP;

    enum VitoWifiState: uint8_t {
      RESET,
      INIT,
      IDLE,
      SYNC,
      SEND,
      RECEIVE,
      RETURN
    } _state;

    union ReturnType{
      int32_t byte4Value; //aal counters are 4 bytes
      int16_t byte2Value; //all temperatures and floats are 2 bytes (factor 10)
      int8_t byte1Value; //statusses and enums are 1 byte
      ReturnType() {
        this->byte4Value = 0;
        }
    } _returnValue;
    uint32_t _timeoutTimer;
    uint32_t _lastMillis;
    uint8_t _errorCount;
    bool _sendMessage;
    bool _sendSync;
    void _resetHandler();
    void _initHandler();
    void _idleHandler();
    void _syncHandler();
    void _sendHandler();
    void _receiveHandler();
    void _returnHandler();

    bool _debugMessage;
    bool _decodeMessage();
    uint8_t _calcChecksum(uint8_t* message, uint8_t lenght);
    bool _checkChecksum(uint8_t* message, uint8_t lenght);
    void _printHex83(uint8_t* data, uint8_t length);
    void _clearInputBuffer();

};
