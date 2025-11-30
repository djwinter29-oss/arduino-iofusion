#include "AnalogScanner.h"
#include <stdlib.h>

AnalogScanner::AnalogScanner(uint8_t channels, uint8_t samples)
  : _channels(channels), _samples(samples) {
  _buf = (int**)malloc(sizeof(int*) * _channels);
  _idx = (uint8_t*)malloc(sizeof(uint8_t) * _channels);
  _sum = (long*)malloc(sizeof(long) * _channels);
  for (uint8_t c = 0; c < _channels; ++c) {
    _buf[c] = (int*)malloc(sizeof(int) * _samples);
    for (uint8_t s = 0; s < _samples; ++s) _buf[c][s] = 0;
    _idx[c] = 0;
    _sum[c] = 0;
  }
  _intervalMs = 100;
  _lastSampleMs = 0;
}

AnalogScanner::~AnalogScanner() {
  if (_buf) {
    for (uint8_t c = 0; c < _channels; ++c) free(_buf[c]);
    free(_buf);
  }
  if (_idx) free(_idx);
  if (_sum) free(_sum);
}

void AnalogScanner::begin() {
  // nothing to do; analogRead is available by default
  // optionally we could seed buffers with initial readings
  for (uint8_t c = 0; c < _channels; ++c) {
    if (_usePullup) {
      pinMode(A0 + c, INPUT_PULLUP);
    }
    int v = analogRead(c);
    for (uint8_t s = 0; s < _samples; ++s) _buf[c][s] = v;
    _sum[c] = (long)v * _samples;
  }
  _lastSampleMs = millis();
}

int AnalogScanner::read(uint8_t idx) {
  if (idx >= _channels) return 0;
  int v = analogRead(idx);
  // subtract oldest, add new
  int out = _buf[idx][_idx[idx]];
  _sum[idx] -= out;
  _buf[idx][_idx[idx]] = v;
  _sum[idx] += v;
  _idx[idx] = (_idx[idx] + 1) % _samples;
  return (int)(_sum[idx] / _samples);
}

int AnalogScanner::readRaw(uint8_t idx) const {
  if (idx >= _channels) return 0;
  return analogRead(idx);
}

float AnalogScanner::readVoltage(uint8_t idx, float vref, uint16_t resolution) {
  int avg = read(idx);
  return ((float)avg * vref) / (float)resolution;
}

void AnalogScanner::setIntervalMs(unsigned long ms) {
  _intervalMs = ms;
}

unsigned long AnalogScanner::getIntervalMs() const { return _intervalMs; }

void AnalogScanner::sampleIfDue() {
  unsigned long now = millis();
  if ((now - _lastSampleMs) < _intervalMs) return;
  _lastSampleMs = now;
  // perform one sample for each channel (push into buffer)
  for (uint8_t idx = 0; idx < _channels; ++idx) {
    int v = analogRead(idx);
    int out = _buf[idx][_idx[idx]];
    _sum[idx] -= out;
    _buf[idx][_idx[idx]] = v;
    _sum[idx] += v;
    _idx[idx] = (_idx[idx] + 1) % _samples;
  }
}

int AnalogScanner::getAverage(uint8_t idx) const {
  if (idx >= _channels) return 0;
  return (int)(_sum[idx] / _samples);
}

void AnalogScanner::setUsePullup(bool v) { _usePullup = v; }
bool AnalogScanner::getUsePullup() const { return _usePullup; }
