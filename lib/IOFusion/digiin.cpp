#include "digiin.h"

DigiIn::DigiIn() {}

bool DigiIn::begin(const uint8_t* pins, uint8_t count, uint16_t windowTicks, float tickHz, bool usePullup) {
  if (count == 0 || count > MAX_PINS) return false;
  _pinCount = count;
  _windowTicks = windowTicks;
  _tickHz = tickHz;
  for (uint8_t i = 0; i < _pinCount; ++i) {
    _pins[i] = pins[i];
    if (usePullup) pinMode(_pins[i], INPUT_PULLUP);
    else pinMode(_pins[i], INPUT);
    _edgeCnt[i] = 0;
    _highCnt[i] = 0;
    _lastState[i] = digitalRead(_pins[i]);
    _freq[i] = 0.0f;
    _duty[i] = 0.0f;
  }
  _samplesInWindow = 0;
  return true;
}

void DigiIn::onTick() {
  // Short ISR-safe sampling
  uint8_t localStates[MAX_PINS];
  for (uint8_t i = 0; i < _pinCount; ++i) {
    localStates[i] = digitalRead(_pins[i]);
  }
  // update counts
  for (uint8_t i = 0; i < _pinCount; ++i) {
    uint8_t s = localStates[i];
    if (s) _highCnt[i]++;
    // detect rising edge
    if (s && !_lastState[i]) _edgeCnt[i]++;
    _lastState[i] = s;
  }
  _samplesInWindow++;
  if (_samplesInWindow >= _windowTicks) {
    // compute results (do minimal floating math in ISR; acceptable for small windows)
    for (uint8_t i = 0; i < _pinCount; ++i) {
      float periodSec = ((float)_windowTicks) / _tickHz; // seconds spanned
      // frequency = edges per window / period
      _freq[i] = (float)_edgeCnt[i] / periodSec;
      _duty[i] = 0.0f;
      if (_samplesInWindow > 0) _duty[i] = (100.0f * (float)_highCnt[i]) / (float)_samplesInWindow;
      // reset counters for next window
      _edgeCnt[i] = 0;
      _highCnt[i] = 0;
    }
    _samplesInWindow = 0;
  }
}

uint8_t DigiIn::getPinCount() const { return _pinCount; }

float DigiIn::getFrequency(uint8_t idx) const {
  if (idx >= _pinCount) return 0.0f;
  noInterrupts();
  float v = _freq[idx];
  interrupts();
  return v;
}

float DigiIn::getDutyCycle(uint8_t idx) const {
  if (idx >= _pinCount) return 0.0f;
  noInterrupts();
  float v = _duty[idx];
  interrupts();
  return v;
}
