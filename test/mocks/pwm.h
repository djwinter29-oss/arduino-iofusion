#ifndef MOCK_PWM_H
#define MOCK_PWM_H

#include <cstdint>

class Timer1PWM {
public:
  Timer1PWM() = default;

  bool begin(float freqHz) {
    _lastBeginFreq = freqHz;
    return _beginOk;
  }

  void setBeginOk(bool ok) { _beginOk = ok; }

  void setDuty(uint8_t channel, float percent) {
    if (channel > 1) return;
    _lastDutyChannel = channel;
    _lastDutyPercent = percent;
  }

  float lastBeginFreq() const { return _lastBeginFreq; }
  uint8_t lastDutyChannel() const { return _lastDutyChannel; }
  float lastDutyPercent() const { return _lastDutyPercent; }

private:
  bool _beginOk = true;
  float _lastBeginFreq = 0.0f;
  uint8_t _lastDutyChannel = 0;
  float _lastDutyPercent = 0.0f;
};

#endif // MOCK_PWM_H
