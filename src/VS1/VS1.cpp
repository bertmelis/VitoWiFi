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
VS1::VS1(const char* interface)
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
  if (_state >= State::SEND) {
    return false;
  }
  if (_currentRequest.createPacket(PacketVS1Type.READ,
                                   datapoint.address(),
                                   datapoint.length()) &&
      _expandResponseBuffer(datapoint.length())) {
    _currentDatapoint = datapoint;
    _requestTime = (_currentMillis != 0) ? _currentMillis : _currentMillis + 1;
    vw_log_i("reading packet OK");
    return true;
  }
  vw_log_i("reading not possible, packet creation error");
  return false;
}

bool VS1::write(const Datapoint& datapoint, const VariantValue& value) {
  if (_state >= State::SEND) {
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
  if (_state >= State::SEND) {
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
    _requestTime = (_currentMillis != 0) ? _currentMillis : _currentMillis + 1;
    vw_log_i("writing packet OK");
    return true;
  }
  vw_log_i("writing not possible, packet creation error");
  return false;
}

bool VS1::begin() {
  _setState(State::INIT);
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

void VS1::_init() {
  if (_interface->available()) {
    if (_interface->read() == 0x05) {
      _setState(State::IDLE);
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
    _setState(State::IDLE);
    _lastMillis = _currentMillis;
  } else {
    _setState(State::INIT);
  }
}

void VS1::_idle() {
  if (_currentDatapoint) {
    _setState(State::SEND);
  } else if (_currentMillis - _lastMillis > 500) {
    if (_interface->write(&VitoWiFiInternals::ProtocolBytes.PROBE[0], 4) == 4) {
      _lastMillis = _currentMillis;
      _setState(State::PROBE_ACK);
    } else {
      _setState(State::INIT);
    }
  }
}

void VS1::_probeAck() {
  if (_interface->available() == 2) {
    _interface->read();
    _interface->read();
    _setState(State::IDLE);
  } else if (_currentMillis - _lastMillis > 1000UL) {
    _setState(State::INIT);
  }
}

void VS1::_send() {
  _bytesTransferred += _interface->write(&_currentRequest[_bytesTransferred], _currentRequest.length() - _bytesTransferred);
  if (_bytesTransferred == _currentRequest.length()) {
    _bytesTransferred = 0;
    _lastMillis = _currentMillis;
    _setState(State::RECEIVE);
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
    _setState(State::IDLE);
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
