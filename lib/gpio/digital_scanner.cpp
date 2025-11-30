// ----- DigitalScanner.cpp -----
#include "digital_scanner.h"

#include <stdlib.h>

// Constructors / destructor
DigitalScanner::DigitalScanner(const uint8_t *pins, uint8_t pinCount, uint8_t samples, float offThresholdHz)
  : _states(nullptr), _pinCount(pinCount), _samples(samples), _offThresholdHz(offThresholdHz), _edgeCb(nullptr) {
  _states = (PinState*)malloc(sizeof(PinState) * _pinCount);
  for (uint8_t i = 0; i < _pinCount; ++i) {
    _states[i].pin = pins[i];
    _states[i].initialized = false;
  }
}

DigitalScanner::DigitalScanner(uint8_t samples, float offThresholdHz)
  : _states(nullptr), _pinCount(0), _samples(samples), _offThresholdHz(offThresholdHz), _edgeCb(nullptr) {
}

DigitalScanner::~DigitalScanner() {
  if (_states) free(_states);
}

// For ISR support we keep a single global reference to the last-created scanner
static DigitalScanner *g_scanner_instance = nullptr;

void DigitalScanner::autoSelectPinsForUno() {
  // On Arduino UNO: digital pins 0-13. Avoid SDA/SCL (A4/A5 are analog pins) and hardware PWM pins (3,5,6,9,10,11 are PWM).
  static const uint8_t candidate[] = {2,4,7,8,12,13};
  if (_states) free(_states);
  _pinCount = sizeof(candidate) / sizeof(candidate[0]);
  _states = (PinState*)malloc(sizeof(PinState) * _pinCount);
  for (uint8_t i = 0; i < _pinCount; ++i) {
    _states[i].pin = candidate[i];
    _states[i].initialized = false;
  }
}

void DigitalScanner::begin() {
  for (uint8_t i = 0; i < _pinCount; ++i) {
    pinMode(_states[i].pin, INPUT_PULLUP);
    _states[i].lastReading = digitalRead(_states[i].pin);
    _states[i].lastEdgeTime = millis();
    _states[i].lastRiseTime = 0;
    _states[i].prevRiseTime = 0;
    _states[i].lastFallTime = 0;
    _states[i].sumPeriodMs = 0;
    _states[i].sumHighMs = 0;
    _states[i].periodCount = 0;
    _states[i].highCount = 0;
    _states[i].freqHz = 0.0f;
    _states[i].dutyPct = 0.0f;
    _states[i].initialized = true;
  }
#if defined(__AVR__)
  uint8_t pcmsk_b = 0;
  uint8_t pcmsk_d = 0;
  for (uint8_t i = 0; i < _pinCount; ++i) {
    uint8_t p = _states[i].pin;
    if (p >= 8 && p <= 13) {
      pcmsk_b |= (1 << (p - 8));
    } else if (p >= 2 && p <= 7) {
      pcmsk_d |= (1 << p);
    }
  }
  if (pcmsk_b) {
    PCMSK0 |= pcmsk_b; // PCINT0..5 -> PORTB
    PCICR |= _BV(PCIE0);
  }
  if (pcmsk_d) {
    PCMSK2 |= pcmsk_d;
    PCICR |= _BV(PCIE2);
  }
#endif
  g_scanner_instance = this;
}

void DigitalScanner::update() {
  unsigned long now = millis();
  for (uint8_t i = 0; i < _pinCount; ++i) {
    PinState &s = _states[i];
    if (!s.initialized) continue;
    int reading = digitalRead(s.pin);
    if (reading != s.lastReading) {
      if (reading == HIGH) {
        s.prevRiseTime = s.lastRiseTime;
        s.lastRiseTime = now;
        if (s.prevRiseTime != 0) {
          unsigned long period = s.lastRiseTime - s.prevRiseTime;
          s.sumPeriodMs += period;
          s.periodCount++;
        }
      } else {
        s.lastFallTime = now;
        if (s.lastRiseTime != 0) {
          unsigned long highMs = s.lastFallTime - s.lastRiseTime;
          s.sumHighMs += highMs;
          s.highCount++;
        }
        if (_edgeCb) _edgeCb(s.pin, false);
      }
      if (reading == HIGH) {
        if (_edgeCb) _edgeCb(s.pin, true);
      }
      s.lastReading = reading;
    }
    if (s.periodCount >= _samples && s.highCount >= _samples) {
      float avgPeriodMs = (float)s.sumPeriodMs / (float)s.periodCount;
      float avgHighMs = (float)s.sumHighMs / (float)s.highCount;
      float hz = 1000.0f / avgPeriodMs;
      float duty = (avgHighMs / avgPeriodMs) * 100.0f;
      if (hz < _offThresholdHz) {
        s.freqHz = 0.0f;
        s.dutyPct = 0.0f;
      } else {
        s.freqHz = hz;
        s.dutyPct = duty;
      }
      s.sumPeriodMs = 0;
      s.sumHighMs = 0;
      s.periodCount = 0;
      s.highCount = 0;
    }
  }
}

void DigitalScanner::setEdgeCallback(EdgeCallback cb) {
  _edgeCb = cb;
}

#if defined(__AVR__)
// ISR implementations for AVR
ISR(PCINT0_vect) {
  if (!g_scanner_instance) return;
  uint8_t changed = PINB;
  DigitalScanner::PinState *states = g_scanner_instance->states();
  uint8_t cnt = g_scanner_instance->pinCount();
  unsigned long now = millis();
  for (uint8_t i = 0; i < cnt; ++i) {
    uint8_t p = states[i].pin;
    if (p < 8 || p > 13) continue;
    uint8_t bit = 1 << (p - 8);
    int reading = (changed & bit) ? HIGH : LOW;
    if (reading != states[i].lastReading) {
      if (reading == HIGH) {
        states[i].prevRiseTime = states[i].lastRiseTime;
        states[i].lastRiseTime = now;
        if (states[i].prevRiseTime != 0) {
          unsigned long period = states[i].lastRiseTime - states[i].prevRiseTime;
          states[i].sumPeriodMs += period;
          states[i].periodCount++;
        }
      } else {
        states[i].lastFallTime = now;
        if (states[i].lastRiseTime != 0) {
          unsigned long highMs = states[i].lastFallTime - states[i].lastRiseTime;
          states[i].sumHighMs += highMs;
          states[i].highCount++;
        }
      }
      if (g_scanner_instance->edgeCallback()) g_scanner_instance->edgeCallback()(p, reading == HIGH);
      states[i].lastReading = reading;
    }
  }
}

ISR(PCINT2_vect) {
  if (!g_scanner_instance) return;
  uint8_t changed = PIND;
  DigitalScanner::PinState *states = g_scanner_instance->states();
  uint8_t cnt = g_scanner_instance->pinCount();
  unsigned long now = millis();
  for (uint8_t i = 0; i < cnt; ++i) {
    uint8_t p = states[i].pin;
    if (p < 2 || p > 7) continue;
    uint8_t bit = 1 << p;
    int reading = (changed & bit) ? HIGH : LOW;
    if (reading != states[i].lastReading) {
      if (reading == HIGH) {
        states[i].prevRiseTime = states[i].lastRiseTime;
        states[i].lastRiseTime = now;
        if (states[i].prevRiseTime != 0) {
          unsigned long period = states[i].lastRiseTime - states[i].prevRiseTime;
          states[i].sumPeriodMs += period;
          states[i].periodCount++;
        }
      } else {
        states[i].lastFallTime = now;
        if (states[i].lastRiseTime != 0) {
          unsigned long highMs = states[i].lastFallTime - states[i].lastRiseTime;
          states[i].sumHighMs += highMs;
          states[i].highCount++;
        }
      }
      if (g_scanner_instance->edgeCallback()) g_scanner_instance->edgeCallback()(p, reading == HIGH);
      states[i].lastReading = reading;
    }
  }
}
#endif

uint8_t DigitalScanner::getPinCount() const { return _pinCount; }
uint8_t DigitalScanner::getPin(uint8_t idx) const { return (idx < _pinCount && _states) ? _states[idx].pin : 0xFF; }
float DigitalScanner::getFrequencyHz(uint8_t idx) const { return (idx < _pinCount && _states) ? _states[idx].freqHz : 0.0f; }
float DigitalScanner::getDutyCyclePercent(uint8_t idx) const { return (idx < _pinCount && _states) ? _states[idx].dutyPct : 0.0f; }
