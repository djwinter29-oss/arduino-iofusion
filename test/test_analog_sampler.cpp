#include <unity.h>

#include "analog_sampler.h"
#include "test_support.h"

void test_analog_sampler_branches() {
  AnalogSampler sampler;
  const uint8_t channels[] = {0, 1};
  const AnalogSampler::Config config = {channels, 2, 5.0f};

  TEST_ASSERT_FALSE(sampler.begin(channels, 0));
  TEST_ASSERT_TRUE(sampler.begin(config));
  TEST_ASSERT_EQUAL_UINT8(2, sampler.getChannelCount());

  sampler.sampleIfDue();
  TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, sampler.getValue(99));

  sampler.setVref(-1.0f);
  sampler.setVref(3.3f);
  sampler.onTick();
  sampler.onTick();
  mockAnalogValues[0] = 1023;
  mockAnalogValues[1] = 256;
  sampler.sampleIfDue();

  TEST_ASSERT_FLOAT_WITHIN(0.02f, 3.3f, sampler.getValue(0));
  TEST_ASSERT_FLOAT_WITHIN(0.02f, (256.0f * 3.3f) / 1023.0f, sampler.getValue(1));
  TEST_ASSERT_EQUAL_UINT16(3300, sampler.getMillivolts(0));
  TEST_ASSERT_EQUAL_UINT32(4, mockAnalogReadCount);
  sampler.sampleIfDue();
  TEST_ASSERT_EQUAL_UINT32(4, mockAnalogReadCount);
  sampler.setVref(70000.0f);
  TEST_ASSERT_EQUAL_UINT16(3300, sampler.getMillivolts(0));
}

void test_analog_sampler_config_edges() {
  AnalogSampler sampler;
  const uint8_t validChannels[] = {0, 5};
  const uint8_t invalidChannels[] = {0, 6};

  TEST_ASSERT_FALSE(sampler.begin(AnalogSampler::Config{nullptr, 1, 5.0f}));
  TEST_ASSERT_FALSE(sampler.begin(AnalogSampler::Config{nullptr, 0, 5.0f}));
  TEST_ASSERT_FALSE(sampler.begin(AnalogSampler::Config{validChannels, 2, 0.0001f}));
  TEST_ASSERT_FALSE(sampler.begin(AnalogSampler::Config{validChannels, 2, 0.0f}));
  TEST_ASSERT_FALSE(sampler.begin(AnalogSampler::Config{validChannels, 2, -1.0f}));
  TEST_ASSERT_FALSE(sampler.begin(AnalogSampler::Config{validChannels, 2, 70000.0f}));
  TEST_ASSERT_EQUAL_UINT8(0, sampler.getChannelCount());
  TEST_ASSERT_FALSE(sampler.begin(nullptr, 2));
  TEST_ASSERT_FALSE(sampler.begin(validChannels, 7));
  TEST_ASSERT_FALSE(sampler.begin(invalidChannels, 2));
  TEST_ASSERT_TRUE(sampler.begin(AnalogSampler::Config{validChannels, 2, 2.5f}));

  sampler.onTick();
  mockAnalogValues[0] = 512;
  mockAnalogValues[5] = 1023;
  sampler.sampleIfDue();

  TEST_ASSERT_FLOAT_WITHIN(0.02f, (512.0f * 2.5f) / 1023.0f, sampler.getValue(0));
  TEST_ASSERT_FLOAT_WITHIN(0.02f, 2.5f, sampler.getValue(1));
  TEST_ASSERT_EQUAL_UINT16(1251, sampler.getMillivolts(0));
  sampler.setVrefMillivolts(0);
  TEST_ASSERT_EQUAL_UINT16(2500, sampler.getMillivolts(1));
}