#pragma once
#include <Arduino.h>
#include "Constants.h"
#include "Helpers/Logger.h"


class OptolinkKW {
  public:
    OptolinkKW();
    //void begin(int8_t rx, int8_t tx);  //software serial
    #ifdef ARDUINO_ARCH_ESP32
    void begin(HardwareSerial* serial, int8_t rxPin, int8_t txPin);  //ESP32
    #endif
    #ifdef ESP8266
    void begin(HardwareSerial* serial);  //ESP8266
    #endif
    void loop();
    const int8_t available() const;
    const bool isBusy() const;
    bool readFromDP(uint16_t address, uint8_t length);
    bool writeToDP(uint16_t address, uint8_t length, uint8_t value[]);
    void read(uint8_t value[]);
    const uint8_t readError();
    void setLogger(Logger* logger);

  private:
    Stream* _stream;
    enum OptolinkState: uint8_t {
      INIT,  //for devices compatible with P300
      IDLE,
      SYNC,
      SEND,
      RECEIVE
    } _state;
    enum OptolinkAction: uint8_t {
      WAIT,
      PROCESS,
      RETURN,
      RETURN_ERROR
    } _action;
    uint16_t _address;
    uint8_t _length;
    bool _writeMessageType;
    uint8_t _value[2];
    uint8_t _rcvBuffer[2];
    uint8_t _rcvBufferLen;
    uint8_t _rcvLen;
    uint32_t _lastMillis;
    uint8_t _numberOfTries;
    uint8_t _errorCode;
    void _initHandler();
    void _idleHandler();
    void _syncHandler();
    void _sendHandler();
    void _receiveHandler();
    bool _debugMessage;
    inline void _printHex(Print* printer, uint8_t array[], uint8_t length);
    inline void _clearInputBuffer();
    Logger* _logger;
};
