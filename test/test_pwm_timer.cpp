#include <unity.h>

#include "pwm.h"
#include "timer.h"
#include "test_support.h"

void test_timer_and_pwm_stubs() {
  Timer2Driver t;
  Timer2Driver other;

  TEST_ASSERT_EQUAL_UINT16(0, t.beginHz(0.0f));
  TEST_ASSERT_EQUAL_UINT16(1, t.beginHz(1000.0f));
  TEST_ASSERT_FALSE(t.attachCallback(nullptr));
  TEST_ASSERT_TRUE(t.attachCallback(timerCallbackA));
  TEST_ASSERT_FALSE(t.attachCallback(timerCallbackA));
  TEST_ASSERT_TRUE(t.attachCallback(timerCallbackB));
  TEST_ASSERT_TRUE(t.attachCallback(timerCallbackC));
  TEST_ASSERT_TRUE(t.attachCallback(timerCallbackD));
  TEST_ASSERT_FALSE(t.attachCallback(timerCallbackE));

  Timer2Driver::handleInterrupt();
  TEST_ASSERT_EQUAL_UINT8(1, gTimerCallbackCountA);
  TEST_ASSERT_EQUAL_UINT8(1, gTimerCallbackCountB);
  TEST_ASSERT_EQUAL_UINT8(1, gTimerCallbackCountC);
  TEST_ASSERT_EQUAL_UINT8(1, gTimerCallbackCountD);
  TEST_ASSERT_EQUAL_UINT8(0, gTimerCallbackCountE);

  TEST_ASSERT_FALSE(other.beginHz(1000.0f));
  TEST_ASSERT_FALSE(other.attachCallback(timerCallbackE));
  TEST_ASSERT_TRUE(t.detachCallback(timerCallbackC));
  TEST_ASSERT_FALSE(t.detachCallback(timerCallbackC));

  Timer2Driver::handleInterrupt();
  TEST_ASSERT_EQUAL_UINT8(2, gTimerCallbackCountA);
  TEST_ASSERT_EQUAL_UINT8(2, gTimerCallbackCountB);
  TEST_ASSERT_EQUAL_UINT8(1, gTimerCallbackCountC);
  TEST_ASSERT_EQUAL_UINT8(2, gTimerCallbackCountD);

  t.stop();
  Timer2Driver::handleInterrupt();
  TEST_ASSERT_EQUAL_UINT8(2, gTimerCallbackCountA);
  TEST_ASSERT_FALSE(t.attachCallback(timerCallbackA));

  TEST_ASSERT_EQUAL_UINT16(1, t.beginHz(1000.0f));
  Timer2Driver::handleInterrupt();
  TEST_ASSERT_EQUAL_UINT8(2, gTimerCallbackCountA);
  TEST_ASSERT_TRUE(t.attachCallback(timerCallbackA));
  Timer2Driver::handleInterrupt();
  TEST_ASSERT_EQUAL_UINT8(3, gTimerCallbackCountA);

  Timer1PWM p;
  TEST_ASSERT_TRUE(p.begin(500.0f));
  p.setDuty(0, -1.0f);
  p.setDuty(1, 120.0f);
  p.stop();
}