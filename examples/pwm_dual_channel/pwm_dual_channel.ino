#include <Arduino.h>

#include "avr_timer1_pwm.h"

namespace {
Timer1PWM pwm;
unsigned long lastStepMs = 0;
float dutyA = 0.0f;
float dutyB = 100.0f;
const Timer1PWM::Config kPwmConfig(1000.0f);
}  // namespace

void setup() {
  Serial.begin(115200);
  delay(100);

  // Timer1 PWM output on Arduino Uno:
  // channel 0 -> pin 9 (OC1A), channel 1 -> pin 10 (OC1B)
  if (!pwm.begin(kPwmConfig)) {
    Serial.println(F("{\"error\":\"pwm init failed\"}"));
    return;
  }

  pwm.setDuty(0, dutyA);
  pwm.setDuty(1, dutyB);
  Serial.println(F("pwm_dual_channel ready"));
}

void loop() {
  unsigned long now = millis();
  if (now - lastStepMs >= 500) {
    lastStepMs = now;

    dutyA += 10.0f;
    if (dutyA > 100.0f) dutyA = 0.0f;
    dutyB = 100.0f - dutyA;

    pwm.setDuty(0, dutyA);
    pwm.setDuty(1, dutyB);

    Serial.print(F("{\"dutyA\":"));
    Serial.print(dutyA, 1);
    Serial.print(F(",\"dutyB\":"));
    Serial.print(dutyB, 1);
    Serial.println(F("}"));
  }
}
