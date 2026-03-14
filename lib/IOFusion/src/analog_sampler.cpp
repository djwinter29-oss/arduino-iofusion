#include "analog_sampler.h"

namespace {

uint16_t scaleAdcToMillivolts(int adcValue, uint16_t vrefMillivolts) {
  if (adcValue <= 0) return 0;
  uint32_t scaled = static_cast<uint32_t>(adcValue) * static_cast<uint32_t>(vrefMillivolts);
  scaled = (scaled + 511U) / 1023U;
  return static_cast<uint16_t>(scaled);
}

bool tryConvertVrefToMillivolts(float vref, uint16_t& millivoltsOut) {
  if (vref <= 0.0f) return false;
  uint32_t millivolts = static_cast<uint32_t>((vref * 1000.0f) + 0.5f);
  if (millivolts == 0 || millivolts > 65535UL) return false;
  millivoltsOut = static_cast<uint16_t>(millivolts);
  return true;
}

}  // namespace

AnalogSampler::AnalogSampler() {
  for (uint8_t i = 0; i < MAX_CHANNELS; ++i) _lastValues[i] = 0;
}

bool AnalogSampler::begin(const Config& config) {
  if (config.channels == nullptr && config.channelCount != 0) return false;
  uint16_t vrefMillivolts = 0;
  if (!tryConvertVrefToMillivolts(config.vref, vrefMillivolts)) return false;
  if (!begin(config.channels, config.channelCount)) return false;
  setVrefMillivolts(vrefMillivolts);
  return true;
}

bool AnalogSampler::begin(const uint8_t* channels, uint8_t channelCount) {
  if (channels == nullptr) return false;
  if (channelCount == 0 || channelCount > MAX_CHANNELS) return false;
  for (uint8_t i = 0; i < channelCount; ++i) {
    uint8_t c = channels[i];
    if (c > 5) return false;  // only A0..A5 on typical AVR
    _channels[i] = c;
  }
  _channelCount = channelCount;
  for (uint8_t ch = 0; ch < _channelCount; ++ch) _lastValues[ch] = 0;
  // Initialize analog input pins (no pinMode for analog pins required on AVR)
  noInterrupts();
  _sampleRequested = false;
  interrupts();
  return true;
}

void AnalogSampler::onTick() {
  // ISR-owned write: loop consumes/clears this flag in sampleIfDue()
  _sampleRequested = true;
}

void AnalogSampler::sampleIfDue() {
  if (!_sampleRequested) return;
  // clear the flag
  noInterrupts();
  _sampleRequested = false;
  interrupts();

  for (uint8_t i = 0; i < _channelCount; ++i) {
    uint8_t ch = _channels[i];
    // Discard first reading after switching channel to allow S/H capacitor to settle.
    (void)analogRead(ch);
    delayMicroseconds(5);
    int v = analogRead(ch);
    _lastValues[i] = v;
  }
}

uint8_t AnalogSampler::getChannelCount() const {
  return _channelCount;
}

float AnalogSampler::getValue(uint8_t idx) const {
  return static_cast<float>(getMillivolts(idx)) / 1000.0f;
}

uint16_t AnalogSampler::getMillivolts(uint8_t idx) const {
  if (idx >= _channelCount) return 0;
  return scaleAdcToMillivolts(_lastValues[idx], _vrefMillivolts);
}

void AnalogSampler::setVref(float vref) {
  uint16_t millivolts = 0;
  if (!tryConvertVrefToMillivolts(vref, millivolts)) return;
  setVrefMillivolts(millivolts);
}

void AnalogSampler::setVrefMillivolts(uint16_t vrefMillivolts) {
  if (vrefMillivolts == 0) return;
  _vrefMillivolts = vrefMillivolts;
}
