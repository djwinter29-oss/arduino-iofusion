#include "encoder_generator.h"

namespace {

bool readControlState(volatile uint8_t* portIn, uint8_t mask, bool activeHigh) {
  bool levelHigh = (portIn && ((*portIn & mask) != 0));
  return activeHigh ? levelHigh : !levelHigh;
}

int32_t saturatingIncrement(int32_t value) {
  if (value == INT32_MAX) return INT32_MAX;
  return value + 1;
}

int32_t saturatingDecrement(int32_t value) {
  if (value == INT32_MIN) return INT32_MIN;
  return value - 1;
}

}  // namespace

bool EncoderGenerator::begin(const Config& config) {
  return begin(config.pinA, config.pinB, config.upPin, config.downPin, config.usePullup,
               config.activeHigh);
}

bool EncoderGenerator::begin(uint8_t pinA, uint8_t pinB, uint8_t up, uint8_t down, bool usePullup,
                             bool activeHigh) {
  if (pinA == pinB) return false;
  uint8_t portA = digitalPinToPort(pinA);
  uint8_t portB = digitalPinToPort(pinB);
  volatile uint8_t* portAOut = portOutputRegister(portA);
  volatile uint8_t* portBOut = portOutputRegister(portB);
  uint8_t maskA = digitalPinToBitMask(pinA);
  uint8_t maskB = digitalPinToBitMask(pinB);
  if (portA == NOT_A_PIN || portB == NOT_A_PIN || portAOut == nullptr || portBOut == nullptr ||
      maskA == 0 || maskB == 0)
    return false;

  uint8_t portUp = digitalPinToPort(up);
  uint8_t portDown = digitalPinToPort(down);
  volatile uint8_t* upPortIn = portInputRegister(portUp);
  volatile uint8_t* downPortIn = portInputRegister(portDown);
  uint8_t upMask = digitalPinToBitMask(up);
  uint8_t downMask = digitalPinToBitMask(down);
  if (portUp == NOT_A_PIN || portDown == NOT_A_PIN || upPortIn == nullptr ||
      downPortIn == nullptr || upMask == 0 || downMask == 0)
    return false;

  _pinA = pinA;
  _pinB = pinB;
  _portAOut = portAOut;
  _portBOut = portBOut;
  _maskA = maskA;
  _maskB = maskB;
  _pinUp = up;
  _pinDown = down;
  _upPortIn = upPortIn;
  _downPortIn = downPortIn;
  _upMask = upMask;
  _downMask = downMask;

  pinMode(_pinA, OUTPUT);
  pinMode(_pinB, OUTPUT);
  if (usePullup) {
    pinMode(_pinUp, INPUT_PULLUP);
    pinMode(_pinDown, INPUT_PULLUP);
  } else {
    pinMode(_pinUp, INPUT);
    pinMode(_pinDown, INPUT);
  }

  *_portAOut &= ~_maskA;
  *_portBOut &= ~_maskB;
  _state = 0;
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
    _position = saturatingIncrement(_position);
    stepped = true;
  } else if (!upHigh && downHigh) {
    _directionUp = false;
    _state = (_state - 1) & 3;
    _position = saturatingDecrement(_position);
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
