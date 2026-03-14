#include <unity.h>

#include "avr_timer1_pwm.h"

void test_avr_timer1_pwm_stubs() {
  Timer1PWM pwm;

  TEST_ASSERT_TRUE(pwm.begin(500.0f));
  pwm.setDuty(0, -1.0f);
  pwm.setDuty(1, 120.0f);
  pwm.stop();
}