#include "avr_timer1_pwm.h"

Timer1PWM::Timer1PWM() {}

bool Timer1PWM::begin(const Config& config) {
  return begin(config.frequencyHz);
}

bool Timer1PWM::begin(float freqHz) {
  return freqHz > 0.0f && freqHz < 1000000.0f;
}

void Timer1PWM::setDuty(uint8_t, float) {}

void Timer1PWM::stop() {}