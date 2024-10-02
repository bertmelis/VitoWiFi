/*
Copyright (c) 2023 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#include "VS1.h"

namespace VitoWiFi {

#if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)
VS1::VS1(HardwareSerial* interface)
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
VS1::VS1(SoftwareSerial* interface)
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
VS1::VS1(const char* interface)
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

VS1::~VS1() {
  delete _interface;
  free(_responseBuffer);
}

void VS1::onResponse(OnResponseCallback callback) {
  _onResponseCallback = callback;
}
void VS1::onError(OnErrorCallback callback) {
  _onErrorCallback = callback;
}

bool VS1::read(const Datapoint& datapoint) {
  if (_currentDatapoint) {
    return false;
  }
  if (_currentRequest.createPacket(PacketVS1Type.READ,
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

bool VS1::write(const Datapoint& datapoint, const VariantValue& value) {
  if (_currentDatapoint) {
    return false;
  }
  uint8_t* payload = reinterpret_cast<uint8_t*>(malloc(datapoint.length()));
  if (!payload) {
    vw_log_i("writing not possible, packet creation error");
    return false;
  }
  datapoint.encode(payload, datapoint.length(), value);
  return write(datapoint, payload, datapoint.length());
}

bool VS1::write(const Datapoint& datapoint, const uint8_t* data, uint8_t length) {
  if (_currentDatapoint) {
    return false;
  }
  if (length != datapoint.length()) {
    vw_log_i("writing not possible, length mismatch");
    return false;
  }
  if (_currentRequest.createPacket(PacketVS1Type.WRITE,
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

bool VS1::begin() {
  if (_interface->begin()) {
    while (_interface->available()) {
      _interface->read();  // clear rx buffer
    }
    _setState(State::INIT);
    return true;
  }
  return false;
}

void VS1::loop() {
  _currentMillis = vw_millis();
  switch (_state) {
  case State::INIT:
    _init();
    break;
  case State::SYNC_ENQ:
    _syncEnq();
    break;
  case State::SYNC_RECV:
    _syncRecv();
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
  if (_currentDatapoint && _currentMillis - _requestTime > 5000UL) {
    _bytesTransferred = 0;
    _setState(State::INIT);
    _tryOnError(OptolinkResult::TIMEOUT);
  }
}

void VS1::end() {
  _interface->end();
  _setState(State::UNDEFINED);
  _currentDatapoint = Datapoint(nullptr, 0x0000, 0, VitoWiFi::noconv);
}

void VS1::_setState(State state) {
  vw_log_i("state %i --> %i", static_cast<std::underlying_type<State>::type>(_state), static_cast<std::underlying_type<State>::type>(state));
  _state = state;
}

// wait for ENQ or reset connection if ENQ is not coming
void VS1::_init() {
  if (_interface->available()) {
    if (_interface->read() == VitoWiFiInternals::ProtocolBytes.ENQ) {
      _lastMillis = _currentMillis;
      _setState(State::SYNC_ENQ);
    }
  } else {
    if (_currentMillis - _lastMillis > 3000UL) {  // reset should Vitotronic be connected with VS2
      _lastMillis = _currentMillis;
      _interface->write(&VitoWiFiInternals::ProtocolBytes.EOT, 1);
    }
  }
}

// if we want to send something within 50msec of receiving the ENQ, send ENQ_ACK and move to SEND
// if > 50msec, return to INIT
void VS1::_syncEnq() {
  if (_currentMillis - _lastMillis < 50) {
    if (_currentDatapoint && _interface->write(&VitoWiFiInternals::ProtocolBytes.ENQ_ACK, 1) == 1) {
      _setState(State::SEND);
      _send();  // speed up things
    }
  } else {
    _setState(State::INIT);
  }
}

// if we want to send something within 50msec of previous SEND, send again
// if > 50msec, return to INIT
void VS1::_syncRecv() {
  if (_currentMillis - _lastMillis < 50) {
    if (_currentDatapoint) {
      _setState(State::SEND);
    }
  } else {
    _setState(State::INIT);
  }
}

// send request and move to RECEIVE
void VS1::_send() {
  _bytesTransferred += _interface->write(&_currentRequest[_bytesTransferred], _currentRequest.length() - _bytesTransferred);
  if (_bytesTransferred == _currentRequest.length()) {
    _bytesTransferred = 0;
    _lastMillis = _currentMillis;
    _setState(State::RECEIVE);
  }
}

// wait for data to receive
// when done, move to SYN_RECV
void VS1::_receive() {
  while (_interface->available()) {
    _responseBuffer[_bytesTransferred] = _interface->read();
    ++_bytesTransferred;
    _lastMillis = _currentMillis;
    vw_log_i("rcv %u / %u", _bytesTransferred, _currentDatapoint.length());
  }
  if (_bytesTransferred == _currentDatapoint.length()) {
    _bytesTransferred = 0;
    _setState(State::SYNC_RECV);
    _tryOnResponse();
  }
}

void VS1::_tryOnResponse() {
  if (_onResponseCallback) {
    _onResponseCallback(_responseBuffer, _currentDatapoint.length(), _currentDatapoint);
  }
  _currentDatapoint = Datapoint(nullptr, 0, 0, noconv);
}

void VS1::_tryOnError(OptolinkResult result) {
  if (_onErrorCallback) {
    _onErrorCallback(result, _currentDatapoint);
  }
  _currentDatapoint = Datapoint(nullptr, 0, 0, noconv);
}

bool VS1::_expandResponseBuffer(uint8_t newSize) {
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
