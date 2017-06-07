#include "Blinker.h"

Blinker::Blinker():
  _lastBlinkPace(0),
  _pin(BUILTIN_LED),
  _on(0)
{}

void Blinker::setLedPin(uint8_t pin, uint8_t on){
  _pin = pin;
  _on = on;
}

void Blinker::start(float blinkPace) {
  if (_lastBlinkPace != blinkPace) {
    _ticker.attach(blinkPace, _tick, _pin);
    _lastBlinkPace = blinkPace;
  }
}

void Blinker::stop() {
  if (_lastBlinkPace != 0) {
    _ticker.detach();
    _lastBlinkPace = 0;
    digitalWrite(_pin, !_on);
  }
}

void Blinker::_tick(uint8_t pin) {
  digitalWrite(pin, !digitalRead(pin));
}
