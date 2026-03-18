#pragma once

#ifdef PIO_UNIT_TESTING
#include "Arduino.h"
#else
#include <Arduino.h>
#endif

class DigitalOut {
 public:
  static constexpr uint8_t MAX_PINS = 8;

  struct Config {
    const uint8_t* pins = nullptr;
    uint8_t pinCount = 0;
    bool initialHigh = false;
  };

  DigitalOut();

  bool begin(const Config& config);
  bool begin(const uint8_t* pins, uint8_t count, bool initialHigh = false);

  bool write(uint8_t idx, bool high);
  bool toggle(uint8_t idx);
  bool setAll(bool high);
  bool getState(uint8_t idx, bool& out) const;
  uint8_t getPinCount() const;

 private:
  uint8_t _pins[MAX_PINS];
  bool _state[MAX_PINS];
  uint8_t _pinCount;
  bool validIndex(uint8_t idx) const;
};
