#include <unity.h>

#include "digiin.h"
#include "test_support.h"

void test_digiin_branches() {
  DigiIn digi;
  const uint8_t badPins[] = {64};
  const uint8_t pins[] = {2};

  TEST_ASSERT_FALSE(digi.begin(pins, 0, 4, 1000.0f, false));
  TEST_ASSERT_FALSE(digi.begin(pins, 1, 0, 1000.0f, false));
  TEST_ASSERT_FALSE(digi.begin(pins, 1, 4, 0.0f, false));
  TEST_ASSERT_FALSE(digi.begin(badPins, 1, 4, 1000.0f, false));
  TEST_ASSERT_TRUE(digi.begin(pins, 1, 4, 1000.0f, false));
  TEST_ASSERT_EQUAL_UINT8(1, digi.getPinCount());

  DigiIn digiPullup;
  TEST_ASSERT_TRUE(digiPullup.begin(pins, 1, 4, 1000.0f, true));
  TEST_ASSERT_EQUAL_UINT8(1, digiPullup.getPinCount());

  digi.updateIfReady();

  setDigitalPin(2, false);
  digi.onTick();
  setDigitalPin(2, true);
  digi.onTick();
  setDigitalPin(2, true);
  digi.onTick();
  setDigitalPin(2, false);
  digi.onTick();

  digi.onTick();
  digi.updateIfReady();

  TEST_ASSERT_FLOAT_WITHIN(0.1f, 250.0f, digi.getFrequency(0));
  TEST_ASSERT_FLOAT_WITHIN(0.1f, 50.0f, digi.getDutyCycle(0));
  TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, digi.getFrequency(9));
  TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, digi.getDutyCycle(9));
}