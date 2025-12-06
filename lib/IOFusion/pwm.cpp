#include "pwm.h"

Timer1PWM::Timer1PWM() {}

bool Timer1PWM::set(uint8_t channel, float freqHz, float percent) {
  if (channel > 1 || freqHz <= 0.0f) return false;
  if (percent < 0.0f) percent = 0.0f;
  if (percent > 100.0f) percent = 100.0f;

  // Determine Timer1 top and prescaler for requested frequency.
  const uint16_t pres[] = {1,8,64,256,1024};
  const uint32_t F = F_CPU;
  uint32_t top = 0;
  uint16_t chosenPres = 1;
  for (uint8_t i = 0; i < sizeof(pres)/sizeof(pres[0]); ++i) {
    float t = ((float)F / (pres[i] * freqHz)) - 1.0f;
    if (t > 0.0f && t <= 65535.0f) {
      top = (uint32_t)(t + 0.5f);
      chosenPres = pres[i];
      break;
    }
  }
  if (top == 0 || top > 65535) return false;

  uint16_t newTop = static_cast<uint16_t>(top);
  uint16_t newPresBits = 0;
  switch (chosenPres) {
    case 1: newPresBits = _BV(CS10); break;
    case 8: newPresBits = _BV(CS11); break;
    case 64: newPresBits = _BV(CS11) | _BV(CS10); break;
    case 256: newPresBits = _BV(CS12); break;
    case 1024: newPresBits = _BV(CS12) | _BV(CS10); break;
    default: newPresBits = _BV(CS10); break;
  }

  bool freqChanged = (!_configured) || (_top != newTop) || (_presBits != newPresBits);

  // Update stored duty before we compute target counts for both channels.
  _dutyPercent[channel] = percent;
  uint16_t targetTop = freqChanged ? newTop : _top;
  uint16_t dutyCounts[2];
  for (uint8_t i = 0; i < 2; ++i) {
    dutyCounts[i] = percentToCounts(_dutyPercent[i], targetTop);
  }

  noInterrupts();
  if (!_configured) {
    pinMode(9, OUTPUT);
    pinMode(10, OUTPUT);
    // Fast PWM, TOP = ICR1 (WGM13:0 = 14), non-inverting on both channels.
    TCCR1A = _BV(COM1A1) | _BV(COM1B1) | _BV(WGM11);
    TCCR1B = _BV(WGM13) | _BV(WGM12);
  }

  if (freqChanged) {
    ICR1 = newTop; // double-buffered, takes effect at BOTTOM
    uint8_t tccr1b = TCCR1B;
    tccr1b &= ~(_BV(CS12) | _BV(CS11) | _BV(CS10));
    tccr1b |= newPresBits;
    TCCR1B = tccr1b;
    _top = newTop;
    _presBits = newPresBits;
  }

  _applyDuty(0, dutyCounts[0]);
  _applyDuty(1, dutyCounts[1]);
  interrupts();

  _configured = true;
  return true;
}

void Timer1PWM::stop() {
  noInterrupts();
  TCCR1A = 0;
  TCCR1B = 0;
  OCR1A = 0;
  OCR1B = 0;
  interrupts();
  pinMode(9, INPUT);
  pinMode(10, INPUT);
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
  uint16_t counts = percentToCounts(percent, _top);
  noInterrupts();
  _applyDuty(channel, counts);
  interrupts();
}

void Timer1PWM::_applyDuty(uint8_t channel, uint16_t value) {
  if (channel == 0) OCR1A = value;
  else if (channel == 1) OCR1B = value;
}

uint16_t Timer1PWM::percentToCounts(float percent, uint16_t top) const {
  if (top == 0) return 0;
  if (percent <= 0.0f) return 0;
  if (percent >= 100.0f) return top;
  uint32_t v = static_cast<uint32_t>((percent / 100.0f) * static_cast<float>(top) + 0.5f);
  if (v > top) v = top;
  return (uint16_t)v;
}
