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
#include "Typedefs.h"

class VitoWifi{
  public:
	  VitoWifi();
	  ~VitoWifi();
	  void begin(HardwareSerial* serial);
    void loop();

    void sendDP(Datapoint* DP);
    void sendDP(Datapoint* DP, uint32_t value);
    CommunicationState getStatus();
    float getValue();

    void setLoggingPrinter(Print* printer);
    Logger& getLogger();
    void enableLogger(bool enable);

  private:
    //general methods and properties
    Logger _logger;
    HardwareSerial* _serial;
    uint8_t _sndBuffer[12];
    uint8_t _sndLen = 0;
    uint8_t _rcvBuffer[12];
    uint8_t _rcvBufferLen = 0;
    uint8_t _rcvLen = 0;

    //connection methods and properties
    void connectionHandler();
    ConnectionState _connectionState;
    uint32_t _timeoutTimer = 0;
    uint32_t _lastMillis = 0;
    uint8_t _errorCount = 0;

    //communication methods and properties
    void communicationHandler();
    CommunicationState _communicationState;
    bool _sendMessage;
    Datapoint* _DP;
    int32_t _value = 0;
    float transform(int32_t value);

    //helper functions
    bool _debugMessage = true;
    bool decodeMessage();
    uint8_t calcChecksum(uint8_t* message, uint8_t lenght);
    bool checkChecksum(uint8_t* message, uint8_t lenght);
    void printHex83(uint8_t* data, uint8_t length);
    void clearInputBuffer();
};
