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
  volatile bool pendingFrameStale;
  bool frameStale;
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
  TEST_ASSERT_FALSE(digitalMonitor.isFrameStale());
  TEST_ASSERT_EQUAL_UINT32(0, digitalMonitor.getFrameSequence());
  TEST_ASSERT_EQUAL_UINT32(1, digitalMonitor.getOverrunCount());
  digitalMonitor.updateIfReady();

  TEST_ASSERT_FLOAT_WITHIN(0.1f, 250.0f, digitalMonitor.getFrequency(0));
  TEST_ASSERT_FLOAT_WITHIN(0.1f, 50.0f, digitalMonitor.getDutyCycle(0));
  TEST_ASSERT_EQUAL_UINT32(250000, digitalMonitor.getFrequencyMilliHz(0));
  TEST_ASSERT_EQUAL_UINT16(500, digitalMonitor.getDutyPermille(0));
  TEST_ASSERT_TRUE(digitalMonitor.isFrameStale());
  TEST_ASSERT_EQUAL_UINT32(1, digitalMonitor.getFrameSequence());
  TEST_ASSERT_EQUAL_UINT32(1, digitalMonitor.getOverrunCount());

  setDigitalPin(2, false);
  digitalMonitor.onTick();
  setDigitalPin(2, true);
  digitalMonitor.onTick();
  setDigitalPin(2, false);
  digitalMonitor.onTick();
  setDigitalPin(2, false);
  digitalMonitor.onTick();
  digitalMonitor.updateIfReady();
  TEST_ASSERT_FALSE(digitalMonitor.isFrameStale());
  TEST_ASSERT_EQUAL_UINT32(2, digitalMonitor.getFrameSequence());
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
  TEST_ASSERT_FALSE(digitalMonitor.isFrameStale());
  digitalMonitor.updateIfReady();

  TEST_ASSERT_FLOAT_WITHIN(0.1f, 500.0f, digitalMonitor.getFrequency(0));
  TEST_ASSERT_FLOAT_WITHIN(0.1f, 100.0f, digitalMonitor.getDutyCycle(0));
  TEST_ASSERT_EQUAL_UINT32(500000, digitalMonitor.getFrequencyMilliHz(0));
  TEST_ASSERT_EQUAL_UINT16(1000, digitalMonitor.getDutyPermille(0));
  TEST_ASSERT_TRUE(digitalMonitor.isFrameStale());
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
  mirror.pendingFrameStale = true;
  mirror.frameStale = true;
  mirror.frameSequence = 3;
  mirror.freqMilliHz[0] = 123000;
  mirror.dutyPermille[0] = 450;

  digitalMonitor.updateIfReady();

  TEST_ASSERT_FLOAT_WITHIN(0.1f, 0.0f, digitalMonitor.getFrequency(0));
  TEST_ASSERT_FLOAT_WITHIN(0.1f, 0.0f, digitalMonitor.getDutyCycle(0));
  TEST_ASSERT_TRUE(digitalMonitor.isFrameStale());
  TEST_ASSERT_EQUAL_UINT32(4, digitalMonitor.getFrameSequence());
  TEST_ASSERT_EQUAL_UINT32(5, digitalMonitor.getOverrunCount());
  TEST_ASSERT_FALSE(mirror.pendingFrameStale);

  mirror.tickMilliHz = 1000000UL;
  mirror.samplesInWindow = 0;
  mirror.windowReady = true;
  mirror.overrunCount = 7;
  mirror.pendingFrameStale = true;
  mirror.frameStale = true;
  mirror.frameSequence = 8;
  mirror.freqMilliHz[0] = 99000;
  mirror.dutyPermille[0] = 770;

  digitalMonitor.updateIfReady();

  TEST_ASSERT_FLOAT_WITHIN(0.1f, 0.0f, digitalMonitor.getFrequency(0));
  TEST_ASSERT_FLOAT_WITHIN(0.1f, 0.0f, digitalMonitor.getDutyCycle(0));
    TEST_ASSERT_TRUE(digitalMonitor.isFrameStale());
  TEST_ASSERT_EQUAL_UINT32(9, digitalMonitor.getFrameSequence());
  TEST_ASSERT_EQUAL_UINT32(7, digitalMonitor.getOverrunCount());
    TEST_ASSERT_FALSE(mirror.pendingFrameStale);

  mirror.tickMilliHz = 0;
  mirror.samplesInWindow = 1;
  mirror.windowReady = true;
    mirror.pendingFrameStale = true;
  mirror.frameStale = true;
  mirror.frameSequence = 0xFFFFFFFFUL;
  digitalMonitor.updateIfReady();
    TEST_ASSERT_TRUE(digitalMonitor.isFrameStale());
  TEST_ASSERT_EQUAL_UINT32(0xFFFFFFFFUL, digitalMonitor.getFrameSequence());

  mirror.tickMilliHz = 1000000UL;
  mirror.samplesInWindow = 1;
  mirror.edgeCnt[0] = 1;
  mirror.highCnt[0] = 1;
  mirror.windowReady = true;
  mirror.pendingFrameStale = true;
  mirror.frameStale = true;
  mirror.frameSequence = 0xFFFFFFFFUL;
  digitalMonitor.updateIfReady();
    TEST_ASSERT_TRUE(digitalMonitor.isFrameStale());
  TEST_ASSERT_EQUAL_UINT32(0xFFFFFFFFUL, digitalMonitor.getFrameSequence());

  mirror.overrunCount = 0xFFFFFFFFUL;
  mirror.windowReady = true;
    mirror.pendingFrameStale = false;
  mirror.frameStale = true;
  digitalMonitor.onTick();
  TEST_ASSERT_TRUE(digitalMonitor.isFrameStale());
  TEST_ASSERT_EQUAL_UINT32(0xFFFFFFFFUL, digitalMonitor.getOverrunCount());
    TEST_ASSERT_TRUE(mirror.pendingFrameStale);

  mirror.pinPortIn[0] = nullptr;
  mirror.windowReady = false;
  digitalMonitor.onTick();
}

void test_digital_input_monitor_copy_frame() {
  DigitalInputMonitor digitalMonitor;
  const uint8_t pins[] = {2, 3};
  TEST_ASSERT_TRUE(digitalMonitor.begin(DigitalInputMonitor::Config{pins, 2, 4, 1000.0f, false}));

  DigitalInputMonitorMirror& mirror = reinterpret_cast<DigitalInputMonitorMirror&>(digitalMonitor);
  mirror.frameSequence = 42;
  mirror.frameStale = true;
  mirror.overrunCount = 77;
  mirror.freqMilliHz[0] = 123000;
  mirror.freqMilliHz[1] = 456000;
  mirror.dutyPermille[0] = 111;
  mirror.dutyPermille[1] = 222;

  DigitalInputMonitor::Frame frame;
  frame.pinCount = 99;
  frame.frameSequence = 99;
  frame.stale = false;
  frame.overrunCount = 99;
  frame.frequencyMilliHz[0] = 1;
  frame.frequencyMilliHz[1] = 1;
  frame.dutyPermille[0] = 1;
  frame.dutyPermille[1] = 1;

  digitalMonitor.copyFrame(frame);

  TEST_ASSERT_EQUAL_UINT8(2, frame.pinCount);
  TEST_ASSERT_EQUAL_UINT32(42, frame.frameSequence);
  TEST_ASSERT_TRUE(frame.stale);
  TEST_ASSERT_EQUAL_UINT32(77, frame.overrunCount);
  TEST_ASSERT_EQUAL_UINT32(123000, frame.frequencyMilliHz[0]);
  TEST_ASSERT_EQUAL_UINT32(456000, frame.frequencyMilliHz[1]);
  TEST_ASSERT_EQUAL_UINT16(111, frame.dutyPermille[0]);
  TEST_ASSERT_EQUAL_UINT16(222, frame.dutyPermille[1]);
}