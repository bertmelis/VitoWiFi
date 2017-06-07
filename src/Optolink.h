#pragma once
#include <Arduino.h>
#include "Helpers/Datatypes.h"

class Optolink {

  public:
    Optolink();
    void begin(HardwareSerial* serial);
    void loop();
    const int8_t available();
    bool readFromDP(uint16_t address, uint8_t length);
    bool writeToDP(uint16_t address, uint8_t length, uint8_t value[]);
    void read(uint8_t value[]);
    void readError(char* errorString, uint8_t length) const;
    void setDebugPrinter(Print* printer);

  private:
    HardwareSerial* _serial;
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
      RECEIVE,
      RETURN
    } _state;
    uint16_t _address;
    uint8_t _length;
    bool _writeMessageType;
    uint8_t _value[4];
    uint8_t _rcvBuffer[12];
    uint8_t _rcvBufferLen;
    uint8_t _rcvLen;
    uint32_t _timeoutTimer;
    uint32_t _lastMillis;
    uint8_t _errorCount;
    uint8_t _errorCode;
    bool _sendMessage;
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
    inline void _printHex83(uint8_t array[], uint8_t length);
    inline void _clearInputBuffer();
    Print* _debugPrinter;
};
