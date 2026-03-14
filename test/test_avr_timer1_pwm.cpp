#include <unity.h>

#include "avr_timer1_pwm.h"

void test_avr_timer1_pwm_stubs() {
  Timer1PWM pwm;

  TEST_ASSERT_TRUE(pwm.begin(Timer1PWM::Config{500.0f}));
  pwm.setDuty(0, -1.0f);
  pwm.setDuty(1, 120.0f);
  pwm.stop();
}

void test_avr_timer1_pwm_config_edges() {
  Timer1PWM pwm;

  TEST_ASSERT_FALSE(pwm.begin(Timer1PWM::Config{0.0f}));
  TEST_ASSERT_FALSE(pwm.begin(Timer1PWM::Config{1000000.0f}));
}