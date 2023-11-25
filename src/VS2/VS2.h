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
#include "../Interface/SerialInterface.h"
#if defined(ARDUINO)
#include "../Interface/HardwareSerialInterface.h"
#include "../Interface/SoftwareSerialInterface.h"
#else
// to include
#endif

namespace VitoWiFi {

class VS2 {
 public:
  typedef std::function<void(const PacketVS2& response, const Datapoint& request)> OnResponseCallback;
  typedef std::function<void(OptolinkResult error, const Datapoint& request)> OnErrorCallback;

  #if defined(ARDUINO)
  explicit VS2(HardwareSerial* interface);
  explicit VS2(SoftwareSerial* interface);
  #else
  // explicit VS2(LinuxSerial* interface);
  #endif
  ~VS2();
  VS2(const VS2&) = delete;

  void onResponse(OnResponseCallback callback);
  void onError(OnErrorCallback callback);

  bool read(const Datapoint& datapoint);
  bool write(const Datapoint& datapoint, const VariantValue& value);

  bool begin();
  void loop();

 private:
  enum class State {
    RESET,
    RESET_ACK,
    INIT,
    INIT_ACK,
    IDLE,
    SENDSTART,
    SENDPACKET,
    SEND_ACK,
    RECEIVE,
    RECEIVE_ACK,
    UNDEFINED
  } _state;
  uint32_t _currentMillis;
  uint32_t _lastMillis;
  uint32_t _requestTime;
  uint8_t _bytesSent;
  VitoWiFiInternals::SerialInterface* _interface;
  VitoWiFiInternals::ParserVS2 _parser;
  Datapoint _currentDatapoint;
  PacketVS2 _currentPacket;
  OnResponseCallback _onResponseCallback;
  OnErrorCallback _onErrorCallback;

  void _reset();
  void _resetAck();
  void _init();
  void _initAck();
  void _idle();
  void _sendStart();
  void _sendPacket();
  void _sendAck();
  void _receive();
  void _receiveAck();

  void _tryOnResponse();
  void _tryOnError(OptolinkResult result);
};

}  // end namespace VitoWiFi
