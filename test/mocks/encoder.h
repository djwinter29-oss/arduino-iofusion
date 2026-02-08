#ifndef MOCK_ENCODER_H
#define MOCK_ENCODER_H

#include <cstdint>

class EncoderGenerator {
public:
  EncoderGenerator() = default;

  void setState(int32_t position, bool directionUp) {
    _position = position;
    _directionUp = directionUp;
  }

  int32_t getPosition() { return _position; }
  bool getDirection() { return _directionUp; }

private:
  int32_t _position = 0;
  bool _directionUp = true;
};

#endif // MOCK_ENCODER_H
