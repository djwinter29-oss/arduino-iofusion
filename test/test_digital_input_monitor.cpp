#include <unity.h>

#include "digital_input_monitor.h"
#include "test_support.h"

void test_digital_input_monitor_branches() {
  DigitalInputMonitor digitalMonitor;
  const uint8_t badPins[] = {64};
  const uint8_t pins[] = {2};
  const DigitalInputMonitor::Config validConfig = {pins, 1, 4, 1000.0f, false};

  TEST_ASSERT_FALSE(digitalMonitor.begin(pins, 0, 4, 1000.0f, false));
  TEST_ASSERT_FALSE(digitalMonitor.begin(pins, 1, 0, 1000.0f, false));
  TEST_ASSERT_FALSE(digitalMonitor.begin(pins, 1, 4, 0.0f, false));
  TEST_ASSERT_FALSE(digitalMonitor.begin(badPins, 1, 4, 1000.0f, false));
  TEST_ASSERT_TRUE(digitalMonitor.begin(validConfig));
  TEST_ASSERT_EQUAL_UINT8(1, digitalMonitor.getPinCount());

  DigitalInputMonitor pullupMonitor;
  TEST_ASSERT_TRUE(pullupMonitor.begin(DigitalInputMonitor::Config{pins, 1, 4, 1000.0f, true}));
  TEST_ASSERT_EQUAL_UINT8(1, pullupMonitor.getPinCount());

  digitalMonitor.updateIfReady();

  setDigitalPin(2, false);
  digitalMonitor.onTick();
  setDigitalPin(2, true);
  digitalMonitor.onTick();
  setDigitalPin(2, true);
  digitalMonitor.onTick();
  setDigitalPin(2, false);
  digitalMonitor.onTick();

  digitalMonitor.onTick();
  digitalMonitor.updateIfReady();

  TEST_ASSERT_FLOAT_WITHIN(0.1f, 250.0f, digitalMonitor.getFrequency(0));
  TEST_ASSERT_FLOAT_WITHIN(0.1f, 50.0f, digitalMonitor.getDutyCycle(0));
  TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, digitalMonitor.getFrequency(9));
  TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, digitalMonitor.getDutyCycle(9));
}