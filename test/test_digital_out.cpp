#include <unity.h>

#include "digital_out.h"

void test_digital_out_begin_rejects_invalid_args() {
  DigitalOut out;
  uint8_t pins[1] = {13};

  TEST_ASSERT_FALSE(out.begin(nullptr, 1));
  TEST_ASSERT_FALSE(out.begin(pins, 0));
  TEST_ASSERT_FALSE(out.begin(pins, DigitalOut::MAX_PINS + 1));
}

void test_digital_out_begin_and_basic_ops() {
  DigitalOut out;
  uint8_t pins[3] = {2, 3, 4};

  TEST_ASSERT_TRUE(out.begin(pins, 3, false));
  TEST_ASSERT_EQUAL_UINT8(3, out.getPinCount());

  bool s = true;
  TEST_ASSERT_TRUE(out.getState(0, s));
  TEST_ASSERT_FALSE(s);

  TEST_ASSERT_TRUE(out.write(0, true));
  TEST_ASSERT_TRUE(out.getState(0, s));
  TEST_ASSERT_TRUE(s);

  TEST_ASSERT_TRUE(out.toggle(0));
  TEST_ASSERT_TRUE(out.getState(0, s));
  TEST_ASSERT_FALSE(s);

  TEST_ASSERT_TRUE(out.setAll(true));
  TEST_ASSERT_TRUE(out.getState(1, s));
  TEST_ASSERT_TRUE(s);
  TEST_ASSERT_TRUE(out.getState(2, s));
  TEST_ASSERT_TRUE(s);
}

void test_digital_out_index_bounds() {
  DigitalOut out;
  uint8_t pins[2] = {5, 6};
  bool s = false;

  TEST_ASSERT_TRUE(out.begin(pins, 2));
  TEST_ASSERT_FALSE(out.write(2, true));
  TEST_ASSERT_FALSE(out.toggle(2));
  TEST_ASSERT_FALSE(out.getState(2, s));
}
