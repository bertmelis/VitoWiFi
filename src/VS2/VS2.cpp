/*
Copyright (c) 2023 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#include "VS2.h"

namespace VitoWiFi {

#if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)
VS2::VS2(HardwareSerial* interface)
: _state(State::UNDEFINED)
, _currentMillis(millis())
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
  _interface = new(std::nothrow) VitoWiFiInternals::HardwareSerialInterface(interface);
  if (!_interface) {
    vw_log_e("Could not create serial interface");
    vw_abort();
  }
}

VS2::VS2(SoftwareSerial* interface)
: _state(State::UNDEFINED)
, _currentMillis(millis())
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
  _interface = new(std::nothrow) VitoWiFiInternals::SoftwareSerialInterface(interface);
  if (!_interface) {
    vw_log_e("Could not create serial interface");
    vw_abort();
  }
}
#else
VS2::VS2(const char* interface)
: _state(State::UNDEFINED)
, _currentMillis(millis())
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
  _interface = new(std::nothrow) VitoWiFiInternals::LinuxSerialInterface(interface);
  if (!_interface) {
    vw_log_e("Could not create serial interface");
    vw_abort();
  }
}
#endif

VS2::~VS2() {
  delete _interface;
}

void VS2::onResponse(OnResponseCallback callback) {
  _onResponseCallback = callback;
}
void VS2::onError(OnErrorCallback callback) {
  _onErrorCallback = callback;
}

bool VS2::read(const Datapoint& datapoint) {
  if (_currentDatapoint) {
    return false;
  }
  if (_currentPacket.createPacket(PacketType::REQUEST,
                                  FunctionCode::READ,
                                  0,
                                  datapoint.address(),
                                  datapoint.length())) {
    _currentDatapoint = datapoint;
    _requestTime = _currentMillis;
    vw_log_i("reading packet OK");
    return true;
  }
  vw_log_i("reading not possible, packet creation error");
  return false;
}

bool VS2::write(const Datapoint& datapoint, const VariantValue& value) {
  if (_currentDatapoint) {
    return false;
  }
  uint8_t* payload = reinterpret_cast<uint8_t*>(malloc(datapoint.length()));
  if (!payload) return false;
  _currentDatapoint.encode(payload, _currentDatapoint.length(), value);
  return write(datapoint, payload, _currentDatapoint.length());
}

bool VS2::write(const Datapoint& datapoint, const uint8_t* data, uint8_t length) {
  if (_currentDatapoint) {
    return false;
  }
  if (length != _currentDatapoint.length()) {
    vw_log_i("writing not possible, length error");
    return false;
  }
  if (_currentPacket.createPacket(PacketType::REQUEST,
                                  FunctionCode::WRITE,
                                  0,
                                  datapoint.address(),
                                  datapoint.length(),
                                  data)) {
    _currentDatapoint = datapoint;
    _requestTime = _currentMillis;
    vw_log_i("writing packet OK");
    return true;
  }
  vw_log_i("writing not possible, packet creation error");
  return false;
}

bool VS2::begin() {
  _setState(State::RESET);
  return _interface->begin();
}

void VS2::loop() {
  _currentMillis = millis();
  switch (_state) {
  case State::RESET:
    _reset();
    break;
  case State::RESET_ACK:
    _resetAck();
    break;
  case State::INIT:
    _init();
    break;
  case State::INIT_ACK:
    _initAck();
    break;
  case State::IDLE:
    _idle();
    break;
  case State::SENDSTART:
    _sendStart();
    break;
  case State::SENDPACKET:
    _sendPacket();
    break;
  case State::SEND_ACK:
    _sendAck();
    break;
  case State::RECEIVE:
    _receive();
    break;
  case State::RECEIVE_ACK:
    _receiveAck();
    break;
  case State::UNDEFINED:
    // begin() not yet called
    break;
  }
  if (_currentDatapoint && _currentMillis - _requestTime > 5000UL) {
    _setState(State::RESET);
    _tryOnError(OptolinkResult::TIMEOUT);
  }
}

void VS2::end() {
  _interface->end();
  _setState(State::UNDEFINED);
  _currentDatapoint = Datapoint(nullptr, 0, 0, noconv);
}

void VS2::_setState(State state) {
  vw_log_i("state %i --> %i", static_cast<std::underlying_type<State>::type>(_state), static_cast<std::underlying_type<State>::type>(state));
  _state = state;
}

void VS2::_reset() {
  while (_interface->available()) _interface->read();
  if (_interface->write(&VitoWiFiInternals::ProtocolBytes.EOT, 1) == 1) {
    _lastMillis = _currentMillis;
    _setState(State::RESET_ACK);
  }
}

void VS2::_resetAck() {
  if (_interface->available()) {
    uint8_t buff = _interface->read();
    if (buff == VitoWiFiInternals::ProtocolBytes.ENQ) {
      _lastMillis = _currentMillis;
      _setState(State::INIT);
    }
  } else {
    if (_currentMillis - _lastMillis > 1000) {
      _setState(State::RESET);
    }
  }
}

void VS2::_init() {
  _bytesSent += _interface->write(&VitoWiFiInternals::ProtocolBytes.SYNC[_bytesSent],
                                  sizeof(VitoWiFiInternals::ProtocolBytes.SYNC) - _bytesSent);
  if (_bytesSent == sizeof(VitoWiFiInternals::ProtocolBytes.SYNC)) {
    _bytesSent = 0;
    _lastMillis = _currentMillis;
    _setState(State::INIT_ACK);
  }
}

void VS2::_initAck() {
  if (_interface->available()) {
    uint8_t buff = _interface->read();
    vw_log_i("rcv: 0x%02x", buff);
    if (buff == VitoWiFiInternals::ProtocolBytes.ACK) {
      _setState(State::IDLE);
    } else {
      _setState(State::RESET);
    }
  } else if (_currentMillis - _lastMillis > 1000) {
    _setState(State::RESET);
  }
}

void VS2::_idle() {
  if (_currentDatapoint) {
    _setState(State::SENDSTART);
  }
  // send INIT every 3 seconds to keep communication alive
  if (_currentMillis - _lastMillis > 3000UL) {
    _setState(State::INIT);
  }
}

void VS2::_sendStart() {
  if (_interface->write(&VitoWiFiInternals::ProtocolBytes.PACKETSTART, 1) == 1) {
    _lastMillis = _currentMillis;
    _setState(State::SENDPACKET);
  }
}

void VS2::_sendPacket() {
  _bytesSent += _interface->write(&_currentPacket[_bytesSent], _currentPacket.length() - _bytesSent);
  if (_bytesSent == _currentPacket.length()) {
    _bytesSent = 0;
    _lastMillis = _currentMillis;
    _setState(State::SEND_ACK);
  }
}

void VS2::_sendAck() {
  if (_interface->available()) {
    uint8_t buff = _interface->read();
    vw_log_i("rcv: 0x%02x", buff);
    if (buff == VitoWiFiInternals::ProtocolBytes.ACK) {  // transmit succesful, moving to next state
      _setState(State::RECEIVE);
    } else if (buff == VitoWiFiInternals::ProtocolBytes.NACK) {  // transmit negatively acknowledged, return to IDLE
      _setState(State::IDLE);
      _tryOnError(OptolinkResult::NACK);
      return;
    }
  }
}

void VS2::_receive() {
  while (_interface->available()) {
    _lastMillis = _currentMillis;
    VitoWiFiInternals::ParserResult result = _parser.parse(_interface->read());
    if (result == VitoWiFiInternals::ParserResult::COMPLETE) {
      _setState(State::RECEIVE_ACK);
      _tryOnResponse();
      return;
    } else if (result == VitoWiFiInternals::ParserResult::CS_ERROR) {
      _setState(State::RESET);
      _tryOnError(OptolinkResult::CRC);
      return;
    } else if (result == VitoWiFiInternals::ParserResult::ERROR) {
      _setState(State::RESET);
      _tryOnError(OptolinkResult::ERROR);
      return;
    }
    // else: continue
  }
}

void VS2::_receiveAck() {
  _interface->write(&VitoWiFiInternals::ProtocolBytes.ACK, 1);
  _lastMillis = _currentMillis;
  _setState(State::IDLE);
}

void VS2::_tryOnResponse() {
  if (_onResponseCallback) {
    _onResponseCallback(_parser.packet(), _currentDatapoint);
  }
  _currentDatapoint = Datapoint(nullptr, 0, 0, noconv);
}

void VS2::_tryOnError(OptolinkResult result) {
  if (_onErrorCallback) {
    _onErrorCallback(result, _currentDatapoint);
  }
  _currentDatapoint = Datapoint(nullptr, 0, 0, noconv);
}

}  // end namespace VitoWiFi
