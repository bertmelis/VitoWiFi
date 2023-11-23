/*
Copyright (c) 2023 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#include "VS2.h"

namespace VitoWiFi {

#if defined(ARDUINO)

VS2::VS2(HardwareSerial* interface)
: _state(State::UNDEFINED)
, _currentMillis(millis())
, _lastMillis(_currentMillis)
, _requestTime(0)
, _bytesSent(0)
, _loopResult(OptolinkResult::CONTINUE)
, _interface(nullptr)
, _parser()
, _currentDatapoint(emptyDatapoint)
, _currentPacket() {
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
, _loopResult(OptolinkResult::CONTINUE)
, _interface(nullptr)
, _parser()
, _currentDatapoint(emptyDatapoint)
, _currentPacket() {
  assert(interface != nullptr);
  _interface = new(std::nothrow) VitoWiFiInternals::SoftwareSerialInterface(interface);
  if (!_interface) {
    vw_log_e("Could not create serial interface");
    vw_abort();
  }
}

#else
// implement Linux
#endif

VS2::~VS2() {
  delete _interface;
}

bool VS2::read(const Datapoint& datapoint) {
  if (_state > State::IDLE) return false;
  if (_currentPacket.createPacket(PacketType::REQUEST,
                                  FunctionCode::READ,
                                  0,
                                  _currentDatapoint.address(),
                                  _currentDatapoint.length())) {
    _currentDatapoint = datapoint;
    _requestTime = (_currentMillis != 0) ? _currentMillis : _currentMillis + 1;
    return true;
  }
  return false;
}

bool VS2::write(const Datapoint& datapoint, const VariantValue& value) {
  if (_state > State::IDLE) return false;
  uint8_t* payload = reinterpret_cast<uint8_t*>(malloc(datapoint.length()));
  if (!payload) return false;
  _currentDatapoint.encode(payload, _currentDatapoint.length(), value);
  if (_currentPacket.createPacket(PacketType::REQUEST,
                                  FunctionCode::WRITE,
                                  0,
                                   _currentDatapoint.address(),
                                   _currentDatapoint.length(),
                                  payload)) {
    _currentDatapoint = datapoint;
    _requestTime = (_currentMillis != 0) ? _currentMillis : _currentMillis + 1;
    return true;
  }
  return false;
}

const PacketVS2& VS2::response() const {
  return _parser.packet();
}

const Datapoint& VS2::datapoint() const {
  return _currentDatapoint;
}

bool VS2::begin() {
  _state = State::RESET;
  return _interface->begin();
}

OptolinkResult VS2::loop() {
  _loopResult = OptolinkResult::CONTINUE;
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
  if (_requestTime != 0 && _currentMillis - _requestTime > 5000UL) {
    _loopResult = OptolinkResult::TIMEOUT;
    _state = State::RESET;
  }
  return _loopResult;
}

void VS2::_reset() {
  while (_interface->available()) _interface->read();
  if (_interface->write(&VitoWiFiInternals::ProtocolBytes.EOT, 1) == 1) {
    _lastMillis = _currentMillis;
    _state = State::RESET_ACK;
  }
}

void VS2::_resetAck() {
  if (_interface->available() && _interface->read() == VitoWiFiInternals::ProtocolBytes.ENQ) {
    _lastMillis = _currentMillis;
    _state = State::INIT;
  } else {
    if (_currentMillis - _lastMillis > 1000) {
      _state = State::RESET;
    }
  }
}

void VS2::_init() {
  _bytesSent += _interface->write(&VitoWiFiInternals::ProtocolBytes.SYNC[_bytesSent],
                                  sizeof(VitoWiFiInternals::ProtocolBytes.SYNC) - _bytesSent);
  if (_bytesSent == sizeof(VitoWiFiInternals::ProtocolBytes.SYNC)) {
    _bytesSent = 0;
    _lastMillis = _currentMillis;
    _state = State::INIT_ACK;
  }
}

void VS2::_initAck() {
  if (_interface->available()) {
    if (_interface->read() == VitoWiFiInternals::ProtocolBytes.ACK) {
      _state = State::IDLE;
    } else {
      _state = State::RESET;
    }
  } else if (_currentMillis - _lastMillis > 1000) {
    _state = State::RESET;
  }
}

void VS2::_idle() {
  if (_currentDatapoint) {
    _state = State::SENDSTART;
  }
  // send INIT every 3 seconds to keep communication alive
  if (_currentMillis - _lastMillis > 3000UL) {
    _state = State::INIT;
  }
}

void VS2::_sendStart() {
  if (_interface->write(&VitoWiFiInternals::ProtocolBytes.PACKETSTART, 1) == 1) {
    _lastMillis = _currentMillis;
    _state = State::SENDPACKET;
  }
}

void VS2::_sendPacket() {
  _bytesSent += _interface->write(&_currentPacket[_bytesSent], _currentPacket.length() - _bytesSent);
  if (_bytesSent == _currentPacket.length()) {
    _bytesSent = 0;
    _lastMillis = _currentMillis;
    _state = State::SEND_ACK;
  }
}

void VS2::_sendAck() {
  if (_interface->available()) {
    uint8_t buff = _interface->read();
    if (buff == 0x06) {  // transmit succesful, moving to next state
      _state = State::RECEIVE;
    } else if (buff == 0x15) {  // transmit negatively acknowledged, return to IDLE
      _loopResult = OptolinkResult::NACK;
      _state = State::IDLE;
      return;
    }
  }
}

void VS2::_receive() {
  while (_interface->available()) {
    _lastMillis = _currentMillis;
    VitoWiFiInternals::ParserResult result = _parser.parse(_interface->read());
    if (result == VitoWiFiInternals::ParserResult::COMPLETE) {
      _loopResult = OptolinkResult::PACKET;
      _state = State::RECEIVE_ACK;
      return;
    } else if (result == VitoWiFiInternals::ParserResult::CS_ERROR) {
      _loopResult = OptolinkResult::CRC;
      _state = State::RESET;
      return;
    } else if (result == VitoWiFiInternals::ParserResult::ERROR) {
      _loopResult = OptolinkResult::ERROR;
      _state = State::RESET;
      return;
    }
    // else: continue
  }
}

void VS2::_receiveAck() {
  _interface->write(&VitoWiFiInternals::ProtocolBytes.ACK, 1);
  _lastMillis = _currentMillis;
  _state = State::IDLE;
}

}  // end namespace VitoWiFi
