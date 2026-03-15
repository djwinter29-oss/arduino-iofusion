#include "digital_out.h"

DigitalOut::DigitalOut() : _pinCount(0) {
  for (uint8_t i = 0; i < MAX_PINS; ++i) {
    _pins[i] = 0;
    _state[i] = false;
  }
}

bool DigitalOut::begin(const Config& config) {
  return begin(config.pins, config.pinCount, config.initialHigh);
}

bool DigitalOut::begin(const uint8_t* pins, uint8_t count, bool initialHigh) {
  if (!pins || count == 0 || count > MAX_PINS) {
    return false;
  }

  _pinCount = count;
  for (uint8_t i = 0; i < _pinCount; ++i) {
    _pins[i] = pins[i];
    _state[i] = initialHigh;
    pinMode(_pins[i], OUTPUT);
    digitalWrite(_pins[i], initialHigh ? HIGH : LOW);
  }

  return true;
}

bool DigitalOut::write(uint8_t idx, bool high) {
  if (!validIndex(idx)) {
    return false;
  }
  _state[idx] = high;
  digitalWrite(_pins[idx], high ? HIGH : LOW);
  return true;
}

bool DigitalOut::toggle(uint8_t idx) {
  if (!validIndex(idx)) {
    return false;
  }
  _state[idx] = !_state[idx];
  digitalWrite(_pins[idx], _state[idx] ? HIGH : LOW);
  return true;
}

bool DigitalOut::setAll(bool high) {
  if (_pinCount == 0) {
    return false;
  }
  for (uint8_t i = 0; i < _pinCount; ++i) {
    _state[i] = high;
    digitalWrite(_pins[i], high ? HIGH : LOW);
  }
  return true;
}

bool DigitalOut::getState(uint8_t idx, bool& out) const {
  if (!validIndex(idx)) {
    return false;
  }
  out = _state[idx];
  return true;
}

uint8_t DigitalOut::getPinCount() const {
  return _pinCount;
}

bool DigitalOut::validIndex(uint8_t idx) const {
  return idx < _pinCount;
}
