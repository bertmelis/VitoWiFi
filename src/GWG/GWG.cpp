/*
Copyright (c) 2023 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#include "GWG.h"

namespace VitoWiFi {

#if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)
GWG::GWG(HardwareSerial* interface)
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
  _interface = new(std::nothrow) VitoWiFiInternals::HardwareSerialInterface(interface);
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

#if defined(ARDUINO_ARCH_ESP8266)
GWG::GWG(SoftwareSerial* interface)
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
  _interface = new(std::nothrow) VitoWiFiInternals::SoftwareSerialInterface(interface);
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
#endif

#else
GWG::GWG(const char* interface)
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
  _interface = new(std::nothrow) VitoWiFiInternals::LinuxSerialInterface(interface);
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
#endif

GWG::~GWG() {
  delete _interface;
  free(_responseBuffer);
}

void GWG::onResponse(OnResponseCallback callback) {
  _onResponseCallback = callback;
}
void GWG::onError(OnErrorCallback callback) {
  _onErrorCallback = callback;
}

bool GWG::read(const Datapoint& datapoint) {
  if (_currentDatapoint) {
    return false;
  }
  if (_currentRequest.createPacket(PacketGWGType.READ,
                                   datapoint.address(),
                                   datapoint.length()) &&
      _expandResponseBuffer(datapoint.length())) {
    _currentDatapoint = datapoint;
    _requestTime = _currentMillis;
    vw_log_i("reading packet OK");
    return true;
  }
  vw_log_i("reading not possible, packet creation error");
  return false;
}

bool GWG::write(const Datapoint& datapoint, const VariantValue& value) {
  if (_currentDatapoint) {
    return false;
  }
  uint8_t* payload = reinterpret_cast<uint8_t*>(malloc(datapoint.length()));
  if (!payload) return false;
  datapoint.encode(payload, datapoint.length(), value);
  bool result = write(datapoint, payload, datapoint.length());
  free(payload);
  return result;
}

bool GWG::write(const Datapoint& datapoint, const uint8_t* data, uint8_t length) {
  if (_currentDatapoint) {
    return false;
  }
  if (length != datapoint.length()) {
    vw_log_i("writing not possible, length mismatch");
    return false;
  }
  if (_currentRequest.createPacket(PacketGWGType.WRITE,
                                   datapoint.address(),
                                   datapoint.length(),
                                   data) &&
      _expandResponseBuffer(datapoint.length())) {
    _currentDatapoint = datapoint;
    _requestTime = _currentMillis;
    vw_log_i("writing packet OK");
    return true;
  }
  vw_log_i("writing not possible, packet creation error");
  return false;
}

bool GWG::begin() {
  _setState(State::INIT);
  return _interface->begin();
}

void GWG::loop() {
  _currentMillis = vw_millis();
  switch (_state) {
  case State::INIT:
    _init();
    break;
  case State::SEND:
    _send();
    break;
  case State::RECEIVE:
    _receive();
    break;
  case State::UNDEFINED:
    // begin() not yet called
    break;
  }
  // double timeout to accomodate for connection initialization
  if (_currentDatapoint && _currentMillis - _requestTime > 3000UL) {
    _setState(State::INIT);
    _tryOnError(OptolinkResult::TIMEOUT);
  }
}

void GWG::end() {
  _interface->end();
  _setState(State::UNDEFINED);
  _currentDatapoint = Datapoint(nullptr, 0x0000, 0, VitoWiFi::noconv);
}

int GWG::getState() const {
  return static_cast<std::underlying_type<State>::type>(_state);
}

bool GWG::isBusy() const {
  if (_currentDatapoint) {
    return true;
  }
  return false;
}

void GWG::_setState(State state) {
  vw_log_i("state %i --> %i", static_cast<std::underlying_type<State>::type>(_state), static_cast<std::underlying_type<State>::type>(state));
  _state = state;
}

void GWG::_init() {
  if (_interface->available()) {
    if (_interface->read() == VitoWiFiInternals::ProtocolBytes.ENQ && _currentDatapoint) {
      _bytesTransferred = 0;
      _setState(State::SEND);
    }
  }
}

void GWG::_send() {
  _bytesTransferred += _interface->write(&_currentRequest[_bytesTransferred], _currentRequest.length() - _bytesTransferred);
  if (_bytesTransferred == _currentRequest.length()) {
    _bytesTransferred = 0;
    _lastMillis = _currentMillis;
    _setState(State::RECEIVE);
  }
}

void GWG::_receive() {
  while (_interface->available()) {
    _responseBuffer[_bytesTransferred] = _interface->read();
    ++_bytesTransferred;
    _lastMillis = _currentMillis;
  }
  if (_bytesTransferred == _currentRequest.length()) {
    _setState(State::INIT);
    _tryOnResponse();
  }
}

void GWG::_tryOnResponse() {
  if (_onResponseCallback) {
    _onResponseCallback(_responseBuffer, _currentRequest.length(), _currentDatapoint);
  }
}

void GWG::_tryOnError(OptolinkResult result) {
  if (_onErrorCallback) {
    _onErrorCallback(result, _currentDatapoint);
  }
  _currentDatapoint = Datapoint(nullptr, 0, 0, noconv);
}

bool GWG::_expandResponseBuffer(uint8_t newSize) {
  if (newSize > _allocatedLength) {
    uint8_t* newBuffer = reinterpret_cast<uint8_t*>(realloc(_responseBuffer, newSize));
    if (!newBuffer) {
      return false;
    }
    _responseBuffer = newBuffer;
    _allocatedLength = newSize;
  }
  return true;
}

}  // end namespace VitoWiFi
