#include <unity.h>

#include "avr_timer2_driver.h"
#include "test_support.h"

void test_avr_timer2_driver_stubs() {
  Timer2Driver timer;
  Timer2Driver other;

  TEST_ASSERT_EQUAL_UINT16(0, timer.beginHz(0.0f));
  TEST_ASSERT_EQUAL_UINT16(1, timer.begin(Timer2Driver::Config{1000.0f}));
  TEST_ASSERT_FALSE(timer.attachCallback(nullptr));
  TEST_ASSERT_TRUE(timer.attachCallback(timerCallbackA));
  TEST_ASSERT_FALSE(timer.attachCallback(timerCallbackA));
  TEST_ASSERT_TRUE(timer.attachCallback(timerCallbackB));
  TEST_ASSERT_TRUE(timer.attachCallback(timerCallbackC));
  TEST_ASSERT_TRUE(timer.attachCallback(timerCallbackD));
  TEST_ASSERT_FALSE(timer.attachCallback(timerCallbackE));

  Timer2Driver::handleInterrupt();
  TEST_ASSERT_EQUAL_UINT8(1, gTimerCallbackCountA);
  TEST_ASSERT_EQUAL_UINT8(1, gTimerCallbackCountB);
  TEST_ASSERT_EQUAL_UINT8(1, gTimerCallbackCountC);
  TEST_ASSERT_EQUAL_UINT8(1, gTimerCallbackCountD);
  TEST_ASSERT_EQUAL_UINT8(0, gTimerCallbackCountE);

  TEST_ASSERT_FALSE(other.begin(Timer2Driver::Config{1000.0f}));
  TEST_ASSERT_FALSE(other.attachCallback(timerCallbackE));
  TEST_ASSERT_TRUE(timer.detachCallback(timerCallbackC));
  TEST_ASSERT_FALSE(timer.detachCallback(timerCallbackC));

  Timer2Driver::handleInterrupt();
  TEST_ASSERT_EQUAL_UINT8(2, gTimerCallbackCountA);
  TEST_ASSERT_EQUAL_UINT8(2, gTimerCallbackCountB);
  TEST_ASSERT_EQUAL_UINT8(1, gTimerCallbackCountC);
  TEST_ASSERT_EQUAL_UINT8(2, gTimerCallbackCountD);

  timer.stop();
  Timer2Driver::handleInterrupt();
  TEST_ASSERT_EQUAL_UINT8(2, gTimerCallbackCountA);
  TEST_ASSERT_FALSE(timer.attachCallback(timerCallbackA));

  TEST_ASSERT_EQUAL_UINT16(1, timer.begin(Timer2Driver::Config{1000.0f}));
  Timer2Driver::handleInterrupt();
  TEST_ASSERT_EQUAL_UINT8(2, gTimerCallbackCountA);
  TEST_ASSERT_TRUE(timer.attachCallback(timerCallbackA));
  Timer2Driver::handleInterrupt();
  TEST_ASSERT_EQUAL_UINT8(3, gTimerCallbackCountA);
}