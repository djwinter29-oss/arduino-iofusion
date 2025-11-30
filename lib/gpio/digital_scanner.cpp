// digital_scanner.cpp
#include "digital_scanner.h"
#include <Arduino.h>
#include <math.h>

DigitalScanner::DigitalScanner() {
  for (auto &c : _ch) {
    c.pin = 0xFF;
    c.lastRead = 0;
    c.lastEdge = 0;
    c.lastRise = 0;
    c.prevRise = 0;
    c.lastFall = 0;
    c.sumPeriod = 0;
    c.sumHigh = 0;
    c.periodCount = 0;
    c.highCount = 0;
    c.freqHz = 0.0f;
    c.dutyPct = 0.0f;
  }
}

void DigitalScanner::addPin(uint8_t pin) {
  if (_pinCount >= DIGITAL_SCANNER_MAX_PINS) return;
  _ch[_pinCount].pin = pin;
  _ch[_pinCount].lastRead = digitalRead(pin);
  _pinCount++;
}

void DigitalScanner::begin() {
  for (uint8_t i = 0; i < _pinCount; ++i) {
    pinMode(_ch[i].pin, INPUT_PULLUP);
    _ch[i].lastRead = digitalRead(_ch[i].pin);
    _ch[i].lastEdge = micros();
    _ch[i].lastRise = 0;
    _ch[i].prevRise = 0;
    _ch[i].lastFall = 0;
    _ch[i].sumPeriod = 0;
    _ch[i].sumHigh = 0;
    _ch[i].periodCount = 0;
    _ch[i].highCount = 0;
    _ch[i].freqHz = 0.0f;
    _ch[i].dutyPct = 0.0f;
  }
}

void DigitalScanner::update() {
  unsigned long now = micros();
  unsigned long nowMs = millis();

  for (uint8_t i = 0; i < _pinCount; ++i) {
    Channel &c = _ch[i];
    uint8_t reading = digitalRead(c.pin);

    if (reading != c.lastRead) {
      unsigned long edgeTime = now;
      if (reading == HIGH) {
        c.prevRise = c.lastRise;
        c.lastRise = edgeTime;
        if (c.prevRise) {
          c.sumPeriod += (c.lastRise - c.prevRise);
          c.periodCount++;
        }
        if (_edgeCb) _edgeCb(c.pin, true);
      } else {
        c.lastFall = edgeTime;
        if (c.lastRise) {
          c.sumHigh += (c.lastFall - c.lastRise);
          c.highCount++;
        }
        if (_edgeCb) _edgeCb(c.pin, false);
      }
      c.lastRead = reading;
      c.lastEdge = edgeTime;
    }

    // Time-based stale data reset
    bool stale = (now - c.lastEdge) > (unsigned long)(2000000UL); // 2s default
    if (_offThresholdHz > 0.1f) {
      unsigned long minPeriodUs = (unsigned long)(1000000.0f / _offThresholdHz);
      stale = (now - c.lastEdge) > (2 * minPeriodUs);
    }

    if (stale) {
      c.freqHz = 0.0f;
      c.dutyPct = 0.0f;
      c.sumPeriod = 0;
      c.sumHigh = 0;
      c.periodCount = 0;
      c.highCount = 0;
      continue;
    }

    // Compute only when enough samples collected
    if (c.periodCount >= _samples && c.highCount >= _samples) {
      if (c.sumPeriod == 0) {
        c.freqHz = 0.0f;
        c.dutyPct = 0.0f;
      } else {
        double avgPeriod = (double)c.sumPeriod / c.periodCount;
        double avgHigh   = (double)c.sumHigh / c.highCount;
        double freq = 1000000.0 / avgPeriod;
        double duty = (avgHigh / avgPeriod) * 100.0;

        if (freq < _offThresholdHz || !isfinite(freq)) {
          c.freqHz = 0.0f;
          c.dutyPct = 0.0f;
        } else {
          c.freqHz = (float)freq;
          c.dutyPct = (float)(duty < 0 ? 0 : (duty > 100 ? 100 : duty));
        }
      }
      // Reset accumulators
      c.sumPeriod = 0;
      c.sumHigh = 0;
      c.periodCount = 0;
      c.highCount = 0;
    }
  }
}

float DigitalScanner::getFrequencyHz(uint8_t idx) const {
  return (idx < _pinCount) ? _ch[idx].freqHz : 0.0f;
}

float DigitalScanner::getDutyCyclePercent(uint8_t idx) const {
  return (idx < _pinCount) ? _ch[idx].dutyPct : 0.0f;
}