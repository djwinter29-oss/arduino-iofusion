#include "encoder_generator.h"

namespace {

bool readControlState(volatile uint8_t* portIn, uint8_t mask, bool activeHigh) {
  bool levelHigh = (portIn && ((*portIn & mask) != 0));
  return activeHigh ? levelHigh : !levelHigh;
}

}  // namespace

bool EncoderGenerator::begin(const Config& config) {
  return begin(config.pinA, config.pinB, config.upPin, config.downPin, config.usePullup,
               config.activeHigh);
}

bool EncoderGenerator::begin(uint8_t pinA, uint8_t pinB, uint8_t up, uint8_t down,
                             bool usePullup, bool activeHigh) {
  if (pinA == pinB) return false;
  _pinA = pinA;
  _pinB = pinB;
  pinMode(_pinA, OUTPUT);
  pinMode(_pinB, OUTPUT);
  uint8_t portA = digitalPinToPort(_pinA);
  uint8_t portB = digitalPinToPort(_pinB);
  _portAOut = portOutputRegister(portA);
  _portBOut = portOutputRegister(portB);
  _maskA = digitalPinToBitMask(_pinA);
  _maskB = digitalPinToBitMask(_pinB);
  if (portA == NOT_A_PIN || portB == NOT_A_PIN || _portAOut == nullptr || _portBOut == nullptr ||
      _maskA == 0 || _maskB == 0)
    return false;
  *_portAOut &= ~_maskA;
  *_portBOut &= ~_maskB;
  _state = 0;

  _pinUp = up;
  _pinDown = down;
  if (usePullup) {
    pinMode(_pinUp, INPUT_PULLUP);
    pinMode(_pinDown, INPUT_PULLUP);
  } else {
    pinMode(_pinUp, INPUT);
    pinMode(_pinDown, INPUT);
  }
  uint8_t portUp = digitalPinToPort(_pinUp);
  uint8_t portDown = digitalPinToPort(_pinDown);
  _upPortIn = portInputRegister(portUp);
  _downPortIn = portInputRegister(portDown);
  _upMask = digitalPinToBitMask(_pinUp);
  _downMask = digitalPinToBitMask(_pinDown);
  if (portUp == NOT_A_PIN || portDown == NOT_A_PIN || _upPortIn == nullptr ||
      _downPortIn == nullptr || _upMask == 0 || _downMask == 0)
    return false;
  _activeHigh = activeHigh;
  noInterrupts();
  _position = 0;
  _directionUp = true;
  interrupts();

  return true;
}

void EncoderGenerator::onTick() {
  // ISR-owned position/state updates; getters read with interrupt guards
  bool upHigh = readControlState(_upPortIn, _upMask, _activeHigh);
  bool downHigh = readControlState(_downPortIn, _downMask, _activeHigh);
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
    if (_portAOut) {
      if (s == 2 || s == 3)
        *_portAOut |= _maskA;
      else
        *_portAOut &= ~_maskA;
    }
    if (_portBOut) {
      if (s == 1 || s == 2)
        *_portBOut |= _maskB;
      else
        *_portBOut &= ~_maskB;
    }
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
  if (_portAOut) *_portAOut &= ~_maskA;
  if (_portBOut) *_portBOut &= ~_maskB;
  interrupts();
}

bool EncoderGenerator::getDirection() {
  noInterrupts();
  bool d = _directionUp;
  interrupts();
  return d;
}
