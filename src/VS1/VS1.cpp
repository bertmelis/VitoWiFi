/*
Copyright (c) 2023 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#include "VS1.h"

namespace VitoWiFi {

#if defined(ARDUINO)

VS1::VS1(HardwareSerial* interface)
: _state(State::UNDEFINED)
, _currentMillis(millis())
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

VS1::VS1(SoftwareSerial* interface)
: _state(State::UNDEFINED)
, _currentMillis(millis())
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

#else
// implement Linux
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
  if (_state >= State::SEND) return false;
  if (_currentRequest.createPacket(PacketVS1Type.READ,
                                   _currentDatapoint.address(),
                                   _currentDatapoint.length()) &&
      _expandResponseBuffer(_currentDatapoint.length())) {
    _currentDatapoint = datapoint;
    _requestTime = (_currentMillis != 0) ? _currentMillis : _currentMillis + 1;
    return true;
  }
  return false;
}

bool VS1::write(const Datapoint& datapoint, const VariantValue& value) {
  if (_state >= State::SEND) return false;
  uint8_t* payload = reinterpret_cast<uint8_t*>(malloc(datapoint.length()));
  if (!payload) return false;
  _currentDatapoint.encode(payload, _currentDatapoint.length(), value);
  return write(datapoint, payload, _currentDatapoint.length());
}

bool VS1::write(const Datapoint& datapoint, const uint8_t* data, uint8_t length) {
  if (_state >= State::SEND) return false;
  if (length != _currentDatapoint.length()) return false;
  if (_currentRequest.createPacket(PacketVS1Type.WRITE,
                                   _currentDatapoint.address(),
                                   _currentDatapoint.length(),
                                   data) &&
      _expandResponseBuffer(_currentDatapoint.length())) {
    _currentDatapoint = datapoint;
    _requestTime = (_currentMillis != 0) ? _currentMillis : _currentMillis + 1;
    return true;
  }
  return false;
}

bool VS1::begin() {
  _state = State::INIT;
  return _interface->begin();
}

void VS1::loop() {
  _currentMillis = millis();
  switch (_state) {
  case State::INIT:
    _init();
    break;
  case State::INIT_ACK:
    _initAck();
    break;
  case State::IDLE:
    _idle();
    break;
  case State::PROBE_ACK:
    _probeAck();
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
  if (_requestTime != 0 && _currentMillis - _requestTime > 4000UL) {
    _state = State::INIT;
    _tryOnError(OptolinkResult::TIMEOUT);
  }
}

void VS1::_init() {
  if (_interface->available()) {
    if (_interface->read() == 0x05) {
      _state = State::IDLE;
    }
  } else {
    if (_currentMillis - _lastMillis > 3000UL) {  // reset should Vitotronic be connected with VS2
      _lastMillis = _currentMillis;
      _interface->write(&VitoWiFiInternals::ProtocolBytes.EOT, 1);
    }
  }
}

void VS1::_initAck() {
  if (_interface->write(&VitoWiFiInternals::ProtocolBytes.ENQ_ACK, 1) == 1) {
    _state = State::IDLE;
    _lastMillis = _currentMillis;
  } else {
    _state = State::INIT;
  }
}

void VS1::_idle() {
  if (_currentDatapoint) {
    _state = State::SEND;
  } else if (_currentMillis - _lastMillis > 500) {
    if (_interface->write(&VitoWiFiInternals::ProtocolBytes.PROBE[0], 4) == 4) {
      _lastMillis = _currentMillis;
      _state = State::PROBE_ACK;
    } else {
      _state = State::INIT;
    }
  }
}

void VS1::_probeAck() {
  if (_interface->available() == 2) {
    _interface->read();
    _interface->read();
    _state = State::IDLE;
  } else if (_currentMillis - _lastMillis > 1000UL) {
    _state = State::INIT;
  }
}

void VS1::_send() {
  _bytesTransferred += _interface->write(&_currentRequest[_bytesTransferred], _currentRequest.length() - _bytesTransferred);
  if (_bytesTransferred == _currentRequest.length()) {
    _bytesTransferred = 0;
    _lastMillis = _currentMillis;
    _state = State::RECEIVE;
  }
}

void VS1::_receive() {
  while (_interface->available()) {
    _responseBuffer[_bytesTransferred] = _interface->read();
    ++_bytesTransferred;
    _lastMillis = _currentMillis;
  }
  if (_bytesTransferred == _currentRequest.length()) {
    _bytesTransferred = 0;
    _state = State::IDLE;
    _tryOnResponse();
  }
}

void VS1::_tryOnResponse() {
  if (_onResponseCallback) _onResponseCallback(_responseBuffer, _currentRequest.length(), _currentDatapoint);
}

void VS1::_tryOnError(OptolinkResult result) {
  if (_onErrorCallback) _onErrorCallback(result, _currentDatapoint);
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
