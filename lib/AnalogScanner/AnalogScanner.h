#ifndef ANALOG_SCANNER_H
#define ANALOG_SCANNER_H

#include <Arduino.h>

class AnalogScanner {
public:
  // samples: number of samples for moving average per channel
  AnalogScanner(uint8_t channels = 6, uint8_t samples = 8);
  ~AnalogScanner();

  // Call once in setup()
  void begin();

  // Background sampling: call `sampleIfDue()` frequently (e.g. in loop()).
  // Set interval in milliseconds; default 100 ms.
  void setIntervalMs(unsigned long ms);
  unsigned long getIntervalMs() const;
  void sampleIfDue();

  // Return current averaged ADC counts without forcing a new sample
  int getAverage(uint8_t idx) const;

  // Call frequently in loop(); returns averaged value for channel idx
  int read(uint8_t idx);

  // Raw single read
  int readRaw(uint8_t idx) const;

  // Return averaged voltage. `vref` default assumes 5.0V, `resolution` default 1023 for 10-bit ADC.
  float readVoltage(uint8_t idx, float vref = 5.0f, uint16_t resolution = 1023);

  uint8_t getChannelCount() const { return _channels; }
  void setUsePullup(bool v);
  bool getUsePullup() const;

private:
  uint8_t _channels;
  uint8_t _samples;
  int **_buf; // per-channel buffers
  uint8_t *_idx; // per-channel indices
  long *_sum; // per-channel sum
  unsigned long _intervalMs;
  unsigned long _lastSampleMs;
  bool _usePullup = false;
};

#endif // ANALOG_SCANNER_H
