/*
Copyright (c) 2023 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#pragma once

#include <functional>

#include "Logging.h"
#include "../Constants.h"
#include "../Helpers.h"
#include "ParserVS2.h"
#include "../Datapoint/Datapoint.h"
#if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)
#include "../Interface/HardwareSerialInterface.h"
#if defined(ARDUINO_ARCH_ESP8266)
#include "../Interface/SoftwareSerialInterface.h"
#endif
#elif defined(__linux__)
#include "../Interface/LinuxSerialInterface.h"
#endif
#include "../Interface/GenericInterface.h"

namespace VitoWiFi {

class VS2 {
 public:
  typedef std::function<void(const PacketVS2& response, const Datapoint& request)> OnResponseCallback;
  typedef std::function<void(OptolinkResult error, const Datapoint& request)> OnErrorCallback;

  #if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)
  explicit VS2(HardwareSerial* interface);
  #if defined(ARDUINO_ARCH_ESP8266)
  explicit VS2(SoftwareSerial* interface);
  #endif
  #else
  explicit VS2(const char* interface);
  #endif
  template<class C>
  explicit VS2(C* interface)
  : _state(State::UNDEFINED)
  , _currentMillis(vw_millis())
  , _lastMillis(_currentMillis)
  , _requestTime(0)
  , _bytesSent(0)
  , _interface(nullptr)
  , _parser()
  , _currentDatapoint(Datapoint(nullptr, 0, 0, noconv))
  , _currentPacket()
  , _onResponseCallback(nullptr)
  , _onErrorCallback(nullptr) {
    assert(interface != nullptr);
    _interface = new(std::nothrow) VitoWiFiInternals::GenericInterface<C>(interface);
    if (!_interface) {
      vw_log_e("Could not create serial interface");
      vw_abort();
    }
  }
  ~VS2();
  VS2(const VS2&) = delete;
  VS2 & operator=(const VS2&) = delete;

  void onResponse(OnResponseCallback callback);
  void onError(OnErrorCallback callback);

  bool read(const Datapoint& datapoint);
  bool write(const Datapoint& datapoint, const VariantValue& value);
  bool write(const Datapoint& datapoint, const uint8_t* data, uint8_t length);

  bool begin();
  void loop();
  void end();

  int getState() const;

 private:
  enum class State {
    RESET,
    RESET_ACK,
    INIT,
    INIT_ACK,
    IDLE,
    SENDSTART,
    SENDPACKET,
    SEND_CRC,
    SEND_ACK,
    RECEIVE,
    RECEIVE_ACK,
    UNDEFINED
  } _state;
  uint32_t _currentMillis;
  uint32_t _lastMillis;
  uint32_t _requestTime;
  uint8_t _bytesTransferred;
  VitoWiFiInternals::SerialInterface* _interface;
  VitoWiFiInternals::ParserVS2 _parser;
  Datapoint _currentDatapoint;
  PacketVS2 _currentPacket;
  OnResponseCallback _onResponseCallback;
  OnErrorCallback _onErrorCallback;

  inline void _setState(State state);

  void _reset();
  void _resetAck();
  void _init();
  void _initAck();
  void _idle();
  void _sendStart();
  void _sendPacket();
  void _sendCRC();
  void _sendAck();
  void _receive();
  void _receiveAck();

  void _tryOnResponse();
  void _tryOnError(OptolinkResult result);
};

}  // end namespace VitoWiFi
