#include "pwm.h"

Timer1PWM::Timer1PWM() {}

bool Timer1PWM::set(uint8_t channel, float freqHz, float percent) {
  if (freqHz <= 0) return false;
  // Try prescalers: 1,8,64,256,1024
  const uint16_t pres[] = {1,8,64,256,1024};
  const uint32_t F = F_CPU;
  uint32_t top = 0;
  uint16_t chosenPres = 1;
  for (uint8_t i = 0; i < sizeof(pres)/sizeof(pres[0]); ++i) {
    float t = ((float)F / (pres[i] * freqHz)) - 1.0f;
    if (t <= 65535.0f) {
      top = (uint32_t)(t + 0.5f);
      chosenPres = pres[i];
      break;
    }
  }
  if (top == 0 || top > 65535) return false;
  _top = (uint16_t)top;
  // configure pins: OC1A (pin 9), OC1B (pin 10)
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  // Fast PWM, TOP = ICR1: WGM13:0 = 14
  TCCR1A = _BV(COM1A1) | _BV(COM1B1) | _BV(WGM11);
  TCCR1B = _BV(WGM13) | _BV(WGM12);
  ICR1 = _top;
  // set prescaler bits
  switch (chosenPres) {
    case 1: _presBits = _BV(CS10); break;
    case 8: _presBits = _BV(CS11); break;
    case 64: _presBits = _BV(CS11) | _BV(CS10); break;
    case 256: _presBits = _BV(CS12); break;
    case 1024: _presBits = _BV(CS12) | _BV(CS10); break;
    default: _presBits = _BV(CS10); break;
  }
  // clear existing CS bits and set new
  TCCR1B = (TCCR1B & ~(_BV(CS12)|_BV(CS11)|_BV(CS10))) | _presBits;
  // apply duty for this channel
  setDuty(channel, percent);
  return true;
}

void Timer1PWM::stop() {
  TCCR1A = 0;
  TCCR1B = 0;
  pinMode(9, INPUT);
  pinMode(10, INPUT);
}

void Timer1PWM::setDuty(uint8_t channel, float percent) {
  if (percent <= 0.0f) {
    _applyDuty(channel, 0);
    return;
  }
  if (percent >= 100.0f) {
    _applyDuty(channel, _top);
    return;
  }
  uint32_t v = (uint32_t)((percent / 100.0f) * (float)_top + 0.5f);
  _applyDuty(channel, (uint16_t)v);
}

void Timer1PWM::_applyDuty(uint8_t channel, uint16_t value) {
  if (channel == 0) OCR1A = value;
  else if (channel == 1) OCR1B = value;
}
