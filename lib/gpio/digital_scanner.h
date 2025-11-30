#ifndef DIGITAL_SCANNER_H
#define DIGITAL_SCANNER_H

#include <Arduino.h>

// Max number of pins to monitor (adjust if needed)
static const uint8_t DIGITAL_SCANNER_MAX_PINS = 8;

class DigitalScanner {
public:
  typedef void (*EdgeCallback)(uint8_t pin, bool rising);

  DigitalScanner();
  void addPin(uint8_t pin);
  void begin();  // call once in setup()
  void update(); // call frequently in loop()

  void setEdgeCallback(EdgeCallback cb) { _edgeCb = cb; }
  void setSamples(uint8_t n) { _samples = n; }
  void setOffThresholdHz(float hz) { _offThresholdHz = hz; }

  // Query results
  uint8_t getPinCount() const { return _pinCount; }
  uint8_t getPin(uint8_t idx) const { return idx < _pinCount ? _ch[idx].pin : 0xFF; }
  float getFrequencyHz(uint8_t idx) const;
  float getDutyCyclePercent(uint8_t idx) const;

private:
  struct Channel {
    uint8_t pin;
    uint8_t lastRead;
    unsigned long lastEdge;
    unsigned long lastRise;
    unsigned long prevRise;
    unsigned long lastFall;

    unsigned long long sumPeriod;
    unsigned long long sumHigh;
    uint16_t periodCount;
    uint16_t highCount;

    float freqHz;
    float dutyPct;
  };

  Channel _ch[DIGITAL_SCANNER_MAX_PINS];
  uint8_t _pinCount = 0;
  uint8_t _samples = 4;
  float _offThresholdHz = 1.0f; // Hz
  EdgeCallback _edgeCb = nullptr;
  unsigned long _lastClear = 0;
  static const unsigned long TIMEOUT_MS = 2000; // 2s fallback
};

#endif