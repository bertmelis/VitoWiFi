#pragma once
#include <Arduino.h>
#include <Ticker.h>

class Blinker {
  public:
    Blinker();
    void setLedPin(uint8_t pin, uint8_t on);
    void start(float blinkPace);
    void stop();

  private:
    uint8_t _pin;
    uint8_t _on;
    Ticker _ticker;
    float _lastBlinkPace;
    static void _tick(uint8_t pin);
};
