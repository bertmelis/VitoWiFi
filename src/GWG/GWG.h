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
#include "PacketGWG.h"
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

class GWG {
 public:
  typedef std::function<void(const uint8_t* data, uint8_t length, const Datapoint& request)> OnResponseCallback;
  typedef std::function<void(OptolinkResult error, const Datapoint& request)> OnErrorCallback;

  #if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)
  explicit GWG(HardwareSerial* interface);
  #if defined(ARDUINO_ARCH_ESP8266)
  explicit GWG(SoftwareSerial* interface);
  #endif
  #else
  explicit GWG(const char* interface);
  #endif
  template<class C>
  GWG(C* interface)
  : _state(State::UNDEFINED)
  , _currentMillis(vw_millis())
  , _lastMillis(_currentMillis)
  , _requestTime(0)
  , _bytesTransferred(0)
  , _interface(nullptr)
  , _currentDatapoint(Datapoint(nullptr, 0x0000, 0, VitoWiFi::noconv))
  , _currentRequest()
  , _responseBuffer(nullptr)
  , _allocatedLength(0)
  , _onResponseCallback(nullptr)
  , _onErrorCallback(nullptr) {
    assert(interface != nullptr);
    _interface = new(std::nothrow) VitoWiFiInternals::GenericInterface(interface);
    if (!_interface) {
      vw_log_e("Could not create serial interface");
      vw_abort();
    }
    _responseBuffer = reinterpret_cast<uint8_t*>(malloc(START_PAYLOAD_LENGTH));
    if (!_responseBuffer) {
      vw_log_e("Could not create response buffer");
      vw_abort();
    }
  }
  ~GWG();
  GWG(const GWG&) = delete;
  GWG & operator=(const GWG&) = delete;

  void onResponse(OnResponseCallback callback);
  void onError(OnErrorCallback callback);

  bool read(const Datapoint& datapoint);
  bool write(const Datapoint& datapoint, const VariantValue& value);
  bool write(const Datapoint& datapoint, const uint8_t* data, uint8_t length);

  bool begin();
  void loop();
  void end();

 private:
  enum class State {
    INIT,
    SEND,
    RECEIVE,
    UNDEFINED
  } _state;
  uint32_t _currentMillis;
  uint32_t _lastMillis;
  uint32_t _requestTime;
  uint8_t _bytesTransferred;
  VitoWiFiInternals::SerialInterface* _interface;
  Datapoint _currentDatapoint;
  PacketGWG _currentRequest;
  uint8_t* _responseBuffer;
  uint8_t _allocatedLength;
  OnResponseCallback _onResponseCallback;
  OnErrorCallback _onErrorCallback;

  inline void _setState(State state);

  void _init();
  void _send();
  void _receive();

  void _tryOnResponse();
  void _tryOnError(OptolinkResult result);

  bool _expandResponseBuffer(uint8_t newSize);
};

}  // end namespace VitoWiFi
