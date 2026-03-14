#include <unity.h>

#include "encoder_generator.h"
#include "test_support.h"

void test_encoder_generator_branches() {
  EncoderGenerator enc;
  TEST_ASSERT_FALSE(enc.begin(9, 9, 2, 3));
  TEST_ASSERT_TRUE(enc.begin(9, 10, 2, 3));

  setDigitalPin(2, true);
  setDigitalPin(3, false);
  enc.onTick();
  enc.onTick();

  setDigitalPin(2, false);
  setDigitalPin(3, true);
  enc.onTick();

  setDigitalPin(2, true);
  setDigitalPin(3, true);
  enc.onTick();

  TEST_ASSERT_EQUAL_INT32(1, enc.getPosition());
  TEST_ASSERT_FALSE(enc.getDirection());

  enc.reset();
  TEST_ASSERT_EQUAL_INT32(0, enc.getPosition());
  TEST_ASSERT_TRUE(enc.getDirection());

  EncoderGenerator activeLowEnc;
  TEST_ASSERT_TRUE(activeLowEnc.begin(9, 10, 2, 3, true, false));

  setDigitalPin(2, false);
  setDigitalPin(3, true);
  activeLowEnc.onTick();
  activeLowEnc.onTick();

  setDigitalPin(2, true);
  setDigitalPin(3, false);
  activeLowEnc.onTick();

  TEST_ASSERT_EQUAL_INT32(1, activeLowEnc.getPosition());
  TEST_ASSERT_FALSE(activeLowEnc.getDirection());
}