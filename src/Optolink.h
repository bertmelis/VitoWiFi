#pragma once
#include <Arduino.h>
#include "Constants.h"
#ifdef USE_SOFTWARESERIAL
#include <SoftwareSerial.h>
#endif

class Optolink {
  public:
    Optolink();
    #ifdef USE_SOFTWARESERIAL
    void begin(int8_t rx, int8_t tx);  //software serial
    #endif
    #ifdef ARDUINO_ARCH_ESP32
    void begin(HardwareSerial* serial, int8_t rxPin, int8_t txPin);  //ESP32
    #endif
    void begin(HardwareSerial* serial);  //ESP8266
    void loop();
    const int8_t available() const;
    const bool isBusy() const;
    bool readFromDP(uint16_t address, uint8_t length);
    bool writeToDP(uint16_t address, uint8_t length, uint8_t value[]);
    void read(uint8_t value[]);
    const uint8_t readError();
    void setDebugPrinter(Print* printer);

  private:
    Stream* _stream;
    enum OptolinkState: uint8_t {
      RESET,
      RESET_ACK,
      INIT,
      INIT_ACK,
      IDLE,
      SYNC,
      SYNC_ACK,
      SEND,
      SEND_ACK,
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
    uint8_t _value[4];
    uint8_t _rcvBuffer[12];
    uint8_t _rcvBufferLen;
    uint8_t _rcvLen;
    uint32_t _lastMillis;
    uint8_t _numberOfTries;
    uint8_t _errorCode;
    void _resetHandler();
    void _resetAckHandler();
    void _initHandler();
    void _initAckHandler();
    void _idleHandler();
    void _syncHandler();
    void _syncAckHandler();
    void _sendHandler();
    void _sendAckHandler();
    void _receiveHandler();
    void _returnHandler();
    bool _debugMessage;
    inline uint8_t _calcChecksum(uint8_t array[], uint8_t length);
    inline bool _checkChecksum(uint8_t array[], uint8_t length);
    inline void _printHex(Print* printer, uint8_t array[], uint8_t length);
    inline void _clearInputBuffer();
    Print* _debugPrinter;
};
