// NOTE: PWM driver is AVR-specific. For native/unit-test builds we provide stubs.

#if defined(UNIT_TEST)
#include "avr_timer1_pwm.h"
#include <stdint.h>

Timer1PWM::Timer1PWM() {}

bool Timer1PWM::begin(float freqHz) {
  return (freqHz > 0.0f && freqHz < 1000000.0f);
}
void Timer1PWM::stop() {}
void Timer1PWM::setDuty(uint8_t, float) {}

#endif

#if !defined(UNIT_TEST)
#include "avr_timer1_pwm.h"

namespace {

constexpr uint8_t kPwmPins[2] = {9, 10};

volatile uint8_t* getPwmPortOut(uint8_t channel) {
  if (channel > 1) return nullptr;
  uint8_t port = digitalPinToPort(kPwmPins[channel]);
  if (port == NOT_A_PIN) return nullptr;
  return portOutputRegister(port);
}

uint8_t getPwmMask(uint8_t channel) {
  if (channel > 1) return 0;
  return digitalPinToBitMask(kPwmPins[channel]);
}

void writePwmPinLevel(uint8_t channel, bool high) {
  volatile uint8_t* portOut = getPwmPortOut(channel);
  uint8_t mask = getPwmMask(channel);
  if (portOut == nullptr || mask == 0) return;
  if (high)
    *portOut |= mask;
  else
    *portOut &= static_cast<uint8_t>(~mask);
}

void setCompareMode(uint8_t channel, bool enabled) {
  if (channel == 0) {
    TCCR1A &= static_cast<uint8_t>(~(_BV(COM1A1) | _BV(COM1A0)));
    if (enabled) TCCR1A |= _BV(COM1A1);
  } else if (channel == 1) {
    TCCR1A &= static_cast<uint8_t>(~(_BV(COM1B1) | _BV(COM1B0)));
    if (enabled) TCCR1A |= _BV(COM1B1);
  }
}

}  // namespace

Timer1PWM::Timer1PWM() {}

bool Timer1PWM::begin(float freqHz) {
  if (freqHz <= 0.0f) return false;

  // Determine Timer1 top and prescaler for requested frequency.
  const uint16_t pres[] = {1, 8, 64, 256, 1024};
  const uint32_t F = F_CPU;
  uint32_t top = 0;
  uint16_t chosenPres = 1;
  for (uint8_t i = 0; i < sizeof(pres) / sizeof(pres[0]); ++i) {
    float t = ((float)F / (pres[i] * freqHz)) - 1.0f;
    if (t > 0.0f && t <= 65535.0f) {
      top = static_cast<uint32_t>(t + 0.5f);
      chosenPres = pres[i];
      break;
    }
  }
  if (top == 0 || top > 65535) return false;

  uint16_t newTop = static_cast<uint16_t>(top);
  uint16_t newPresBits = 0;
  switch (chosenPres) {
    case 1:
      newPresBits = _BV(CS10);
      break;
    case 8:
      newPresBits = _BV(CS11);
      break;
    case 64:
      newPresBits = _BV(CS11) | _BV(CS10);
      break;
    case 256:
      newPresBits = _BV(CS12);
      break;
    case 1024:
      newPresBits = _BV(CS12) | _BV(CS10);
      break;
    default:
      newPresBits = _BV(CS10);
      break;
  }

  noInterrupts();
  pinMode(kPwmPins[0], OUTPUT);
  pinMode(kPwmPins[1], OUTPUT);

  // Fast PWM, TOP = ICR1 (mode 14). Per-channel output mode is applied below.
  TCCR1A = _BV(WGM11);
  uint8_t tccr1b = _BV(WGM13) | _BV(WGM12);
  TCCR1B = tccr1b;

  ICR1 = newTop;
  _applyDuty(0, _dutyPercent[0], newTop);
  _applyDuty(1, _dutyPercent[1], newTop);
  TCNT1 = 0;

  tccr1b |= newPresBits;
  TCCR1B = tccr1b;

  _top = newTop;
  _presBits = newPresBits;
  _configured = true;
  interrupts();

  return true;
}

void Timer1PWM::stop() {
  noInterrupts();
  TCCR1A = 0;
  TCCR1B = 0;
  OCR1A = 0;
  OCR1B = 0;
  writePwmPinLevel(0, false);
  writePwmPinLevel(1, false);
  interrupts();
  pinMode(kPwmPins[0], INPUT);
  pinMode(kPwmPins[1], INPUT);
  _top = 0;
  _presBits = 0;
  _dutyPercent[0] = 0.0f;
  _dutyPercent[1] = 0.0f;
  _configured = false;
}

void Timer1PWM::setDuty(uint8_t channel, float percent) {
  if (channel > 1) return;
  if (percent < 0.0f) percent = 0.0f;
  if (percent > 100.0f) percent = 100.0f;
  _dutyPercent[channel] = percent;
  if (!_configured || _top == 0) return;
  noInterrupts();
  _applyDuty(channel, percent, _top);
  interrupts();
}

void Timer1PWM::_applyDuty(uint8_t channel, float percent, uint16_t top) {
  if (channel > 1) return;

  if (percent <= 0.0f) {
    setCompareMode(channel, false);
    if (channel == 0)
      OCR1A = 0;
    else
      OCR1B = 0;
    writePwmPinLevel(channel, false);
    return;
  }

  if (percent >= 100.0f) {
    setCompareMode(channel, false);
    if (channel == 0)
      OCR1A = top;
    else
      OCR1B = top;
    writePwmPinLevel(channel, true);
    return;
  }

  uint16_t value = percentToCounts(percent, top);
  if (channel == 0)
    OCR1A = value;
  else
    OCR1B = value;
  setCompareMode(channel, true);
}

uint16_t Timer1PWM::percentToCounts(float percent, uint16_t top) const {
  if (top == 0) return 0;
  if (percent <= 0.0f) return 0;
  if (percent >= 100.0f) return top;
  uint32_t v = static_cast<uint32_t>((percent / 100.0f) * static_cast<float>(top) + 0.5f);
  if (v > top) v = top;
  return (uint16_t)v;
}
#endif  // !UNIT_TEST
