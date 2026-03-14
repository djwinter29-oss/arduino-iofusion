#include <unity.h>

#include "digital_input_monitor.h"
#include "test_support.h"

namespace {

struct DigitalInputMonitorMirror {
  uint8_t pins[8];
  uint8_t pinCount;
  uint16_t windowTicks;
  uint32_t tickMilliHz;
  volatile uint8_t* pinPortIn[8];
  uint8_t pinMask[8];
  volatile uint16_t samplesInWindow;
  volatile uint16_t edgeCnt[8];
  volatile uint16_t highCnt[8];
  volatile uint8_t lastState[8];
  volatile bool windowReady;
  volatile uint32_t overrunCount;
  volatile bool frameStale;
  uint32_t frameSequence;
  uint32_t freqMilliHz[8];
  uint16_t dutyPermille[8];
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
  TEST_ASSERT_FALSE(digitalMonitor.isFrameStale());
  TEST_ASSERT_EQUAL_UINT32(0, digitalMonitor.getFrameSequence());

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
  TEST_ASSERT_TRUE(digitalMonitor.isFrameStale());
  TEST_ASSERT_EQUAL_UINT32(0, digitalMonitor.getFrameSequence());
  TEST_ASSERT_EQUAL_UINT32(1, digitalMonitor.getOverrunCount());
  digitalMonitor.updateIfReady();

  TEST_ASSERT_FLOAT_WITHIN(0.1f, 250.0f, digitalMonitor.getFrequency(0));
  TEST_ASSERT_FLOAT_WITHIN(0.1f, 50.0f, digitalMonitor.getDutyCycle(0));
  TEST_ASSERT_EQUAL_UINT32(250000, digitalMonitor.getFrequencyMilliHz(0));
  TEST_ASSERT_EQUAL_UINT16(500, digitalMonitor.getDutyPermille(0));
  TEST_ASSERT_FALSE(digitalMonitor.isFrameStale());
  TEST_ASSERT_EQUAL_UINT32(1, digitalMonitor.getFrameSequence());
  TEST_ASSERT_EQUAL_UINT32(1, digitalMonitor.getOverrunCount());
  TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, digitalMonitor.getFrequency(9));
  TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, digitalMonitor.getDutyCycle(9));
}

void test_digital_input_monitor_config_edges() {
  DigitalInputMonitor digitalMonitor;
  const uint8_t pins[] = {2};
  const uint8_t mixedPins[] = {2, 64};

  TEST_ASSERT_FALSE(digitalMonitor.begin(DigitalInputMonitor::Config{nullptr, 1, 4, 1000.0f,
                                                                     false}));
  TEST_ASSERT_FALSE(digitalMonitor.begin(DigitalInputMonitor::Config{nullptr, 0, 4, 1000.0f,
                                                                     false}));
  TEST_ASSERT_FALSE(digitalMonitor.begin(nullptr, 1, 4, 1000.0f, false));
  TEST_ASSERT_FALSE(digitalMonitor.begin(pins, 9, 4, 1000.0f, false));
  TEST_ASSERT_FALSE(digitalMonitor.begin(mixedPins, 2, 4, 1000.0f, false));
  TEST_ASSERT_EQUAL_UINT8(0, digitalMonitor.getPinCount());
  TEST_ASSERT_EQUAL_HEX8(0xFF, mockPinModes[2]);
  mockNullInputPort = digitalPinToPort(2);
  TEST_ASSERT_FALSE(digitalMonitor.begin(pins, 1, 4, 1000.0f, false));
  mockNullInputPort = -1;
  mockZeroMaskPin = 2;
  TEST_ASSERT_FALSE(digitalMonitor.begin(pins, 1, 4, 1000.0f, false));
  mockZeroMaskPin = -1;
  TEST_ASSERT_TRUE(digitalMonitor.begin(DigitalInputMonitor::Config{pins, 1, 2, 1000.0f, false}));
  TEST_ASSERT_EQUAL_UINT32(0, digitalMonitor.getOverrunCount());
  TEST_ASSERT_FALSE(digitalMonitor.isFrameStale());
  TEST_ASSERT_EQUAL_UINT32(0, digitalMonitor.getFrameSequence());

  setDigitalPin(2, true);
  digitalMonitor.onTick();
  digitalMonitor.onTick();
  digitalMonitor.onTick();
  TEST_ASSERT_EQUAL_UINT32(1, digitalMonitor.getOverrunCount());
  digitalMonitor.updateIfReady();

  TEST_ASSERT_FLOAT_WITHIN(0.1f, 500.0f, digitalMonitor.getFrequency(0));
  TEST_ASSERT_FLOAT_WITHIN(0.1f, 100.0f, digitalMonitor.getDutyCycle(0));
  TEST_ASSERT_EQUAL_UINT32(500000, digitalMonitor.getFrequencyMilliHz(0));
  TEST_ASSERT_EQUAL_UINT16(1000, digitalMonitor.getDutyPermille(0));
  TEST_ASSERT_FALSE(digitalMonitor.isFrameStale());
  TEST_ASSERT_EQUAL_UINT32(1, digitalMonitor.getFrameSequence());

  const uint8_t laterBadPins[] = {3, 64};
  TEST_ASSERT_FALSE(digitalMonitor.begin(laterBadPins, 2, 4, 1000.0f, false));
  TEST_ASSERT_EQUAL_UINT8(1, digitalMonitor.getPinCount());
  TEST_ASSERT_EQUAL_HEX8(0xFF, mockPinModes[3]);

  DigitalInputMonitorMirror& mirror = reinterpret_cast<DigitalInputMonitorMirror&>(digitalMonitor);
  mirror.pinCount = 1;
  mirror.tickMilliHz = 0;
  mirror.samplesInWindow = 1;
  mirror.windowReady = true;
  mirror.overrunCount = 5;
  mirror.frameStale = true;
  mirror.frameSequence = 3;
  mirror.freqMilliHz[0] = 123000;
  mirror.dutyPermille[0] = 450;

  digitalMonitor.updateIfReady();

  TEST_ASSERT_FLOAT_WITHIN(0.1f, 0.0f, digitalMonitor.getFrequency(0));
  TEST_ASSERT_FLOAT_WITHIN(0.1f, 0.0f, digitalMonitor.getDutyCycle(0));
  TEST_ASSERT_FALSE(digitalMonitor.isFrameStale());
  TEST_ASSERT_EQUAL_UINT32(4, digitalMonitor.getFrameSequence());
  TEST_ASSERT_EQUAL_UINT32(5, digitalMonitor.getOverrunCount());

  mirror.tickMilliHz = 1000000UL;
  mirror.samplesInWindow = 0;
  mirror.windowReady = true;
  mirror.overrunCount = 7;
  mirror.frameStale = true;
  mirror.frameSequence = 8;
  mirror.freqMilliHz[0] = 99000;
  mirror.dutyPermille[0] = 770;

  digitalMonitor.updateIfReady();

  TEST_ASSERT_FLOAT_WITHIN(0.1f, 0.0f, digitalMonitor.getFrequency(0));
  TEST_ASSERT_FLOAT_WITHIN(0.1f, 0.0f, digitalMonitor.getDutyCycle(0));
  TEST_ASSERT_FALSE(digitalMonitor.isFrameStale());
  TEST_ASSERT_EQUAL_UINT32(9, digitalMonitor.getFrameSequence());
  TEST_ASSERT_EQUAL_UINT32(7, digitalMonitor.getOverrunCount());

  mirror.tickMilliHz = 0;
  mirror.samplesInWindow = 1;
  mirror.windowReady = true;
  mirror.frameStale = true;
  mirror.frameSequence = 0xFFFFFFFFUL;
  digitalMonitor.updateIfReady();
  TEST_ASSERT_FALSE(digitalMonitor.isFrameStale());
  TEST_ASSERT_EQUAL_UINT32(0xFFFFFFFFUL, digitalMonitor.getFrameSequence());

  mirror.tickMilliHz = 1000000UL;
  mirror.samplesInWindow = 1;
  mirror.edgeCnt[0] = 1;
  mirror.highCnt[0] = 1;
  mirror.windowReady = true;
  mirror.frameStale = true;
  mirror.frameSequence = 0xFFFFFFFFUL;
  digitalMonitor.updateIfReady();
  TEST_ASSERT_FALSE(digitalMonitor.isFrameStale());
  TEST_ASSERT_EQUAL_UINT32(0xFFFFFFFFUL, digitalMonitor.getFrameSequence());

  mirror.overrunCount = 0xFFFFFFFFUL;
  mirror.windowReady = true;
  mirror.frameStale = true;
  digitalMonitor.onTick();
  TEST_ASSERT_TRUE(digitalMonitor.isFrameStale());
  TEST_ASSERT_EQUAL_UINT32(0xFFFFFFFFUL, digitalMonitor.getOverrunCount());

  mirror.pinPortIn[0] = nullptr;
  mirror.windowReady = false;
  digitalMonitor.onTick();
}