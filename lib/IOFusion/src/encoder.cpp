#include "encoder.h"

// Clean, instance-based implementation of EncoderGenerator.

bool EncoderGenerator::begin(uint8_t pinA, uint8_t pinB, uint8_t up, uint8_t down) {
  if (pinA == pinB) return false;
  _pinA = pinA;
  _pinB = pinB;
  pinMode(_pinA, OUTPUT);
  pinMode(_pinB, OUTPUT);
  digitalWrite(_pinA, LOW);
  digitalWrite(_pinB, LOW);
  _state = 0;

  _pinUp = up;
  _pinDown = down;
  pinMode(_pinUp, INPUT_PULLUP);
  pinMode(_pinDown, INPUT_PULLUP);
  noInterrupts();
  _position = 0;
  _directionUp = true;
  interrupts();

  return true;
}

void EncoderGenerator::onTick() {
  // Level-triggered sampling: if up is HIGH and down LOW -> step forward each tick
  bool upHigh = digitalRead(_pinUp) == HIGH;
  bool downHigh = digitalRead(_pinDown) == HIGH;
  bool stepped = false;
  if (upHigh && !downHigh) {
    _directionUp = true;
    _state = (_state + 1) & 3;
    _position++;
    stepped = true;
  } else if (!upHigh && downHigh) {
    _directionUp = false;
    _state = (_state - 1) & 3;
    _position--;
    stepped = true;
  } else {
    // both low or both high: do nothing
  }
  // write outputs once if a step occurred
  if (stepped) {
    uint8_t s = _state;
    digitalWrite(_pinA, (s == 2 || s == 3) ? HIGH : LOW);
    digitalWrite(_pinB, (s == 1 || s == 2) ? HIGH : LOW);
  }
}

int32_t EncoderGenerator::getPosition() {
  noInterrupts();
  int32_t v = _position;
  interrupts();
  return v;
}

void EncoderGenerator::reset() {
  noInterrupts();
  _position = 0;
  _state = 0;
  _directionUp = true;
  // set outputs to known idle (both LOW)
  digitalWrite(_pinA, LOW);
  digitalWrite(_pinB, LOW);
  interrupts();
}

bool EncoderGenerator::getDirection() {
  noInterrupts();
  bool d = _directionUp;
  interrupts();
  return d;
}
