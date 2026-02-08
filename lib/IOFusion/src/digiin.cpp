#include "digiin.h"

DigiIn::DigiIn() {}

bool DigiIn::begin(const uint8_t* pins, uint8_t count, uint16_t windowTicks, float tickHz, bool usePullup) {
  if (count == 0 || count > MAX_PINS) return false;
  if (windowTicks == 0 || tickHz <= 0.0f) return false;
  _pinCount = count;
  _windowTicks = windowTicks;
  _tickHz = tickHz;
  for (uint8_t i = 0; i < _pinCount; ++i) {
    _pins[i] = pins[i];
    if (usePullup) pinMode(_pins[i], INPUT_PULLUP);
    else pinMode(_pins[i], INPUT);
    uint8_t port = digitalPinToPort(_pins[i]);
    _pinPortIn[i] = portInputRegister(port);
    _pinMask[i] = digitalPinToBitMask(_pins[i]);
    if (port == NOT_A_PIN || _pinPortIn[i] == nullptr || _pinMask[i] == 0) return false;
    _edgeCnt[i] = 0;
    _highCnt[i] = 0;
    _lastState[i] = (_pinPortIn[i] && ((*_pinPortIn[i] & _pinMask[i]) != 0)) ? 1 : 0;
    _freq[i] = 0.0f;
    _duty[i] = 0.0f;
  }
  _samplesInWindow = 0;
  _windowReady = false;
  return true;
}

void DigiIn::onTick() {
  // Short ISR-safe sampling
  if (_windowReady) return;
  for (uint8_t i = 0; i < _pinCount; ++i) {
    uint8_t s = (_pinPortIn[i] && ((*_pinPortIn[i] & _pinMask[i]) != 0)) ? 1 : 0;
    if (s) _highCnt[i]++;
    // detect rising edge
    if (s && !_lastState[i]) _edgeCnt[i]++;
    _lastState[i] = s;
  }
  _samplesInWindow++;
  if (_samplesInWindow >= _windowTicks) {
    _windowReady = true;
  }
}

void DigiIn::updateIfReady() {
  if (!_windowReady) return;

  uint16_t samples = 0;
  uint16_t edgeCnt[MAX_PINS];
  uint16_t highCnt[MAX_PINS];

  noInterrupts();
  samples = _samplesInWindow;
  for (uint8_t i = 0; i < _pinCount; ++i) {
    edgeCnt[i] = _edgeCnt[i];
    highCnt[i] = _highCnt[i];
    _edgeCnt[i] = 0;
    _highCnt[i] = 0;
  }
  _samplesInWindow = 0;
  _windowReady = false;
  interrupts();

  if (samples == 0 || _tickHz <= 0.0f) {
    for (uint8_t i = 0; i < _pinCount; ++i) {
      _freq[i] = 0.0f;
      _duty[i] = 0.0f;
    }
    return;
  }

  float periodSec = ((float)samples) / _tickHz;
  for (uint8_t i = 0; i < _pinCount; ++i) {
    _freq[i] = (float)edgeCnt[i] / periodSec;
    _duty[i] = (100.0f * (float)highCnt[i]) / (float)samples;
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
