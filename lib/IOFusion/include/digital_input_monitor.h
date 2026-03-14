// Sampled digital input frequency and duty monitor
#ifndef IOFUSION_DIGITAL_INPUT_MONITOR_H
#define IOFUSION_DIGITAL_INPUT_MONITOR_H

#include <Arduino.h>

class DigitalInputMonitor {
 public:
  DigitalInputMonitor();
  // Begin monitoring pins: `pins` array of `count` pin numbers (max 8).
  // `windowTicks` is the number of onTick() samples per measurement window.
  // `tickHz` is the frequency (Hz) at which onTick() will be called.
  // This is a sampled estimator, not a hardware capture block: pulses shorter
  // than one tick may be missed, and transitions above Nyquist will alias.
  bool begin(const uint8_t* pins, uint8_t count, uint16_t windowTicks = 1000,
             float tickHz = 1000.0f, bool usePullup = false);

  void onTick();
  void updateIfReady();

  uint8_t getPinCount() const;
  float getFrequency(uint8_t idx) const;
  float getDutyCycle(uint8_t idx) const;

 private:
  static const uint8_t MAX_PINS = 8;
  uint8_t _pins[MAX_PINS];
  uint8_t _pinCount = 0;
  uint16_t _windowTicks = 1000;
  float _tickHz = 1000.0f;
  volatile uint8_t* _pinPortIn[MAX_PINS];
  uint8_t _pinMask[MAX_PINS];
  volatile uint16_t _samplesInWindow = 0;
  volatile uint16_t _edgeCnt[MAX_PINS];
  volatile uint16_t _highCnt[MAX_PINS];
  volatile uint8_t _lastState[MAX_PINS];
  volatile bool _windowReady = false;
  float _freq[MAX_PINS];
  float _duty[MAX_PINS];
};

#endif  // IOFUSION_DIGITAL_INPUT_MONITOR_H