#ifndef MOCK_DIGIIN_H
#define MOCK_DIGIIN_H

#include <cstdint>

class DigiIn {
public:
  DigiIn() = default;

  void setValues(const float* freq, const float* duty, uint8_t count) {
    _count = count;
    for (uint8_t i = 0; i < count && i < kMax; ++i) {
      _freq[i] = freq[i];
      _duty[i] = duty[i];
    }
  }

  uint8_t getPinCount() const { return _count; }

  float getFrequency(uint8_t idx) const {
    if (idx >= _count) return 0.0f;
    return _freq[idx];
  }

  float getDutyCycle(uint8_t idx) const {
    if (idx >= _count) return 0.0f;
    return _duty[idx];
  }

private:
  static constexpr uint8_t kMax = 8;
  float _freq[kMax] = {0.0f};
  float _duty[kMax] = {0.0f};
  uint8_t _count = 0;
};

#endif // MOCK_DIGIIN_H
