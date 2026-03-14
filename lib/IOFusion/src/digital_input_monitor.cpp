#include "digital_input_monitor.h"

namespace {

inline uint8_t readPinState(volatile uint8_t* portIn, uint8_t mask) {
  return (portIn && ((*portIn & mask) != 0)) ? 1 : 0;
}

uint32_t hzToMilliHz(float tickHz) {
  if (tickHz <= 0.0f) return 0;
  return static_cast<uint32_t>((tickHz * 1000.0f) + 0.5f);
}

}  // namespace

DigitalInputMonitor::DigitalInputMonitor() {}

bool DigitalInputMonitor::begin(const Config& config) {
  if (config.pins == nullptr && config.pinCount != 0) return false;
  return begin(config.pins, config.pinCount, config.windowTicks, config.tickHz,
               config.usePullup);
}

bool DigitalInputMonitor::begin(const uint8_t* pins, uint8_t count, uint16_t windowTicks,
                                float tickHz, bool usePullup) {
  uint32_t tickMilliHz = hzToMilliHz(tickHz);
  if (pins == nullptr) return false;
  if (count == 0 || count > MAX_PINS) return false;
  if (windowTicks == 0 || tickMilliHz == 0) return false;
  _pinCount = count;
  _windowTicks = windowTicks;
  _tickMilliHz = tickMilliHz;
  for (uint8_t i = 0; i < _pinCount; ++i) {
    _pins[i] = pins[i];
    if (usePullup)
      pinMode(_pins[i], INPUT_PULLUP);
    else
      pinMode(_pins[i], INPUT);
    uint8_t port = digitalPinToPort(_pins[i]);
    _pinPortIn[i] = portInputRegister(port);
    _pinMask[i] = digitalPinToBitMask(_pins[i]);
    if (port == NOT_A_PIN || _pinPortIn[i] == nullptr || _pinMask[i] == 0) return false;
    _edgeCnt[i] = 0;
    _highCnt[i] = 0;
    _lastState[i] = readPinState(_pinPortIn[i], _pinMask[i]);
    _freqMilliHz[i] = 0;
    _dutyPermille[i] = 0;
  }
  _samplesInWindow = 0;
  _windowReady = false;
  _overrunCount = 0;
  return true;
}

void DigitalInputMonitor::onTick() {
  if (_windowReady) {
    if (_overrunCount != 0xFFFFFFFFUL) {
      ++_overrunCount;
    }
    return;
  }
  for (uint8_t i = 0; i < _pinCount; ++i) {
    uint8_t s = readPinState(_pinPortIn[i], _pinMask[i]);
    if (s) _highCnt[i]++;
    if (s && !_lastState[i]) _edgeCnt[i]++;
    _lastState[i] = s;
  }
  _samplesInWindow++;
  if (_samplesInWindow >= _windowTicks) {
    _windowReady = true;
  }
}

void DigitalInputMonitor::updateIfReady() {
  if (!_windowReady) return;

  uint16_t samples = 0;
  uint16_t edgeCnt[MAX_PINS];
  uint16_t highCnt[MAX_PINS];
  uint32_t tickMilliHz = 0;

  noInterrupts();
  samples = _samplesInWindow;
  tickMilliHz = _tickMilliHz;
  for (uint8_t i = 0; i < _pinCount; ++i) {
    edgeCnt[i] = _edgeCnt[i];
    highCnt[i] = _highCnt[i];
    _edgeCnt[i] = 0;
    _highCnt[i] = 0;
  }
  _samplesInWindow = 0;
  _windowReady = false;
  interrupts();

  if (samples == 0 || tickMilliHz == 0) {
    for (uint8_t i = 0; i < _pinCount; ++i) {
      _freqMilliHz[i] = 0;
      _dutyPermille[i] = 0;
    }
    return;
  }

  for (uint8_t i = 0; i < _pinCount; ++i) {
    uint64_t freqMilliHz = static_cast<uint64_t>(edgeCnt[i]) * static_cast<uint64_t>(tickMilliHz);
    _freqMilliHz[i] = static_cast<uint32_t>((freqMilliHz + (samples / 2U)) / samples);
    uint32_t dutyPermille = (static_cast<uint32_t>(highCnt[i]) * 1000U) + (samples / 2U);
    _dutyPermille[i] = static_cast<uint16_t>(dutyPermille / samples);
  }
}

uint8_t DigitalInputMonitor::getPinCount() const {
  return _pinCount;
}

float DigitalInputMonitor::getFrequency(uint8_t idx) const {
  return static_cast<float>(getFrequencyMilliHz(idx)) / 1000.0f;
}

uint32_t DigitalInputMonitor::getFrequencyMilliHz(uint8_t idx) const {
  if (idx >= _pinCount) return 0;
  noInterrupts();
  uint32_t v = _freqMilliHz[idx];
  interrupts();
  return v;
}

float DigitalInputMonitor::getDutyCycle(uint8_t idx) const {
  return static_cast<float>(getDutyPermille(idx)) / 10.0f;
}

uint16_t DigitalInputMonitor::getDutyPermille(uint8_t idx) const {
  if (idx >= _pinCount) return 0;
  noInterrupts();
  uint16_t v = _dutyPermille[idx];
  interrupts();
  return v;
}

uint32_t DigitalInputMonitor::getOverrunCount() const {
  noInterrupts();
  uint32_t v = _overrunCount;
  interrupts();
  return v;
}