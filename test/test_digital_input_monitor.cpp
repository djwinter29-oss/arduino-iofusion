#include <unity.h>

#include "digital_input_monitor.h"
#include "test_support.h"

namespace {

struct DigitalInputMonitorMirror {
  uint8_t pins[8];
  uint8_t pinCount;
  uint16_t windowTicks;
  float tickHz;
  volatile uint8_t* pinPortIn[8];
  uint8_t pinMask[8];
  volatile uint16_t samplesInWindow;
  volatile uint16_t edgeCnt[8];
  volatile uint16_t highCnt[8];
  volatile uint8_t lastState[8];
  volatile bool windowReady;
  float freq[8];
  float duty[8];
};

}  // namespace

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

void test_digital_input_monitor_config_edges() {
  DigitalInputMonitor digitalMonitor;
  const uint8_t pins[] = {2};

  TEST_ASSERT_FALSE(digitalMonitor.begin(DigitalInputMonitor::Config{nullptr, 1, 4, 1000.0f,
                                                                     false}));
  TEST_ASSERT_FALSE(digitalMonitor.begin(DigitalInputMonitor::Config{nullptr, 0, 4, 1000.0f,
                                                                     false}));
  TEST_ASSERT_FALSE(digitalMonitor.begin(nullptr, 1, 4, 1000.0f, false));
  TEST_ASSERT_FALSE(digitalMonitor.begin(pins, 9, 4, 1000.0f, false));
  mockNullInputPort = digitalPinToPort(2);
  TEST_ASSERT_FALSE(digitalMonitor.begin(pins, 1, 4, 1000.0f, false));
  mockNullInputPort = -1;
  mockZeroMaskPin = 2;
  TEST_ASSERT_FALSE(digitalMonitor.begin(pins, 1, 4, 1000.0f, false));
  mockZeroMaskPin = -1;
  TEST_ASSERT_TRUE(digitalMonitor.begin(DigitalInputMonitor::Config{pins, 1, 2, 1000.0f, false}));

  setDigitalPin(2, true);
  digitalMonitor.onTick();
  digitalMonitor.onTick();
  digitalMonitor.onTick();
  digitalMonitor.updateIfReady();

  TEST_ASSERT_FLOAT_WITHIN(0.1f, 500.0f, digitalMonitor.getFrequency(0));
  TEST_ASSERT_FLOAT_WITHIN(0.1f, 100.0f, digitalMonitor.getDutyCycle(0));

  DigitalInputMonitorMirror& mirror = reinterpret_cast<DigitalInputMonitorMirror&>(digitalMonitor);
  mirror.pinCount = 1;
  mirror.tickHz = 0.0f;
  mirror.samplesInWindow = 1;
  mirror.windowReady = true;
  mirror.freq[0] = 123.0f;
  mirror.duty[0] = 45.0f;

  digitalMonitor.updateIfReady();

  TEST_ASSERT_FLOAT_WITHIN(0.1f, 0.0f, digitalMonitor.getFrequency(0));
  TEST_ASSERT_FLOAT_WITHIN(0.1f, 0.0f, digitalMonitor.getDutyCycle(0));

  mirror.tickHz = 1000.0f;
  mirror.samplesInWindow = 0;
  mirror.windowReady = true;
  mirror.freq[0] = 99.0f;
  mirror.duty[0] = 77.0f;

  digitalMonitor.updateIfReady();

  TEST_ASSERT_FLOAT_WITHIN(0.1f, 0.0f, digitalMonitor.getFrequency(0));
  TEST_ASSERT_FLOAT_WITHIN(0.1f, 0.0f, digitalMonitor.getDutyCycle(0));

  mirror.pinPortIn[0] = nullptr;
  mirror.windowReady = false;
  digitalMonitor.onTick();
}