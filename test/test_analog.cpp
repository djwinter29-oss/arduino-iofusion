#include <unity.h>

#include "analog.h"

void test_analog_sampler_branches() {
  AnalogSampler sampler;
  const uint8_t channels[] = {0, 1};

  TEST_ASSERT_FALSE(sampler.begin(channels, 0));
  TEST_ASSERT_TRUE(sampler.begin(channels, 2));
  TEST_ASSERT_EQUAL_UINT8(2, sampler.getChannelCount());

  sampler.sampleIfDue();
  TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, sampler.getValue(99));

  sampler.setVref(-1.0f);
  sampler.setVref(3.3f);
  sampler.onTick();
  mockAnalogValues[0] = 1023;
  mockAnalogValues[1] = 256;
  sampler.sampleIfDue();

  TEST_ASSERT_FLOAT_WITHIN(0.02f, 3.3f, sampler.getValue(0));
  TEST_ASSERT_FLOAT_WITHIN(0.02f, (256.0f * 3.3f) / 1023.0f, sampler.getValue(1));
}