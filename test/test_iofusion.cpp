#include <unity.h>

#include "Arduino.h"
#include "analog.h"
#include "cmdline.h"
#include "digiin.h"
#include "encoder.h"
#include "pwm.h"
#include "timer.h"

namespace {
void setDigitalPin(uint8_t pin, bool high) {
  uint8_t port = digitalPinToPort(pin);
  uint8_t mask = digitalPinToBitMask(pin);
  if (port == NOT_A_PIN) return;
  if (high)
    mockPortIn[port] |= mask;
  else
    mockPortIn[port] &= static_cast<uint8_t>(~mask);
}

void clearPorts() {
  for (uint8_t i = 0; i < 8; ++i) {
    mockPortIn[i] = 0;
    mockPortOut[i] = 0;
  }
}

void runCmd(CmdLine& cli, const char* cmd) {
  Serial.clearOutput();
  Serial.setInput(std::string(cmd) + "\n");
  cli.processSerial();
}
}  // namespace

void setUp() {
  mockMillis = 0;
  for (int i = 0; i < 16; ++i) mockAnalogValues[i] = 0;
  clearPorts();
  Serial.clearOutput();
  Serial.setInput("");
}

void tearDown() {}

void test_analog_sampler_branches() {
  AnalogSampler sampler;
  const uint8_t channels[] = {0, 1};

  TEST_ASSERT_FALSE(sampler.begin(channels, 0));
  TEST_ASSERT_TRUE(sampler.begin(channels, 2));
  TEST_ASSERT_EQUAL_UINT8(2, sampler.getChannelCount());

  sampler.sampleIfDue();  // no-op path
  TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, sampler.getValue(99));

  sampler.setVref(-1.0f);  // ignored path
  sampler.setVref(3.3f);   // positive path
  sampler.onTick();
  mockAnalogValues[0] = 1023;
  mockAnalogValues[1] = 256;
  sampler.sampleIfDue();

  TEST_ASSERT_FLOAT_WITHIN(0.02f, 3.3f, sampler.getValue(0));
  TEST_ASSERT_FLOAT_WITHIN(0.02f, (256.0f * 3.3f) / 1023.0f, sampler.getValue(1));
}

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

  // cover usePullup branch too
  DigiIn digiPullup;
  TEST_ASSERT_TRUE(digiPullup.begin(pins, 1, 4, 1000.0f, true));
  TEST_ASSERT_EQUAL_UINT8(1, digiPullup.getPinCount());

  digi.updateIfReady();  // !_windowReady path

  setDigitalPin(2, false);
  digi.onTick();
  setDigitalPin(2, true);
  digi.onTick();
  setDigitalPin(2, true);
  digi.onTick();
  setDigitalPin(2, false);
  digi.onTick();

  // extra tick while windowReady should be ignored
  digi.onTick();

  digi.updateIfReady();

  TEST_ASSERT_FLOAT_WITHIN(0.1f, 250.0f, digi.getFrequency(0));
  TEST_ASSERT_FLOAT_WITHIN(0.1f, 50.0f, digi.getDutyCycle(0));
  TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, digi.getFrequency(9));
  TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, digi.getDutyCycle(9));
}

void test_encoder_branches() {
  EncoderGenerator enc;
  TEST_ASSERT_FALSE(enc.begin(9, 9, 2, 3));
  TEST_ASSERT_TRUE(enc.begin(9, 10, 2, 3));

  // forward
  setDigitalPin(2, true);
  setDigitalPin(3, false);
  enc.onTick();
  enc.onTick();

  // backward
  setDigitalPin(2, false);
  setDigitalPin(3, true);
  enc.onTick();

  // no-step branch (both high)
  setDigitalPin(2, true);
  setDigitalPin(3, true);
  enc.onTick();

  TEST_ASSERT_EQUAL_INT32(1, enc.getPosition());
  TEST_ASSERT_FALSE(enc.getDirection());

  enc.reset();
  TEST_ASSERT_EQUAL_INT32(0, enc.getPosition());
  TEST_ASSERT_TRUE(enc.getDirection());
}

void test_cmdline_commands() {
  AnalogSampler analog;
  DigiIn digi;
  EncoderGenerator encoder;
  Timer1PWM pwm;

  const uint8_t aPins[] = {0};
  const uint8_t dPins[] = {2};
  TEST_ASSERT_TRUE(analog.begin(aPins, 1));
  TEST_ASSERT_TRUE(digi.begin(dPins, 1, 4, 1000.0f, false));
  TEST_ASSERT_TRUE(encoder.begin(9, 10, 2, 3));

  CmdLine cli(analog, digi, encoder, pwm, aPins, 1, dPins, 1);

  runCmd(cli, "help");
  TEST_ASSERT_NOT_NULL(strstr(Serial.getOutput().c_str(), "help"));

  runCmd(cli, "analog?");
  TEST_ASSERT_NOT_NULL(strstr(Serial.getOutput().c_str(), "\"a0\""));

  runCmd(cli, "digital?");
  TEST_ASSERT_NOT_NULL(strstr(Serial.getOutput().c_str(), "\"d2\""));

  runCmd(cli, "encoder?");
  TEST_ASSERT_NOT_NULL(strstr(Serial.getOutput().c_str(), "encoder"));

  runCmd(cli, "pwm-freq");
  TEST_ASSERT_NOT_NULL(strstr(Serial.getOutput().c_str(), "missing frequency"));

  runCmd(cli, "pwm-freq abc");
  TEST_ASSERT_NOT_NULL(strstr(Serial.getOutput().c_str(), "invalid frequency"));

  runCmd(cli, "pwm-freq 1000");
  TEST_ASSERT_NOT_NULL(strstr(Serial.getOutput().c_str(), "status"));

  runCmd(cli, "pwm-freq 1000000");
  TEST_ASSERT_NOT_NULL(strstr(Serial.getOutput().c_str(), "unable to set frequency"));

  runCmd(cli, "pwm-duty");
  TEST_ASSERT_NOT_NULL(strstr(Serial.getOutput().c_str(), "missing duty parameters"));

  runCmd(cli, "pwm-duty x 50");
  TEST_ASSERT_NOT_NULL(strstr(Serial.getOutput().c_str(), "invalid channel"));

  runCmd(cli, "pwm-duty 3 50");
  TEST_ASSERT_NOT_NULL(strstr(Serial.getOutput().c_str(), "invalid channel"));

  runCmd(cli, "pwm-duty 1 abc");
  TEST_ASSERT_NOT_NULL(strstr(Serial.getOutput().c_str(), "invalid duty"));

  runCmd(cli, "pwm-duty 1 12.5");
  TEST_ASSERT_NOT_NULL(strstr(Serial.getOutput().c_str(), "status"));

  runCmd(cli, "unknown-cmd");
  TEST_ASSERT_NOT_NULL(strstr(Serial.getOutput().c_str(), "unknown command"));

  // cover idle-timeout dispatch path (no trailing newline)
  Serial.clearOutput();
  advanceMillis(1);
  Serial.setInput("help");
  cli.processSerial();
  TEST_ASSERT_EQUAL_UINT32(
      0, static_cast<uint32_t>(strstr(Serial.getOutput().c_str(), "help") != nullptr));
  advanceMillis(100);
  cli.processSerial();
  TEST_ASSERT_NOT_NULL(strstr(Serial.getOutput().c_str(), "help"));
}

void test_timer_and_pwm_stubs() {
  Timer2Driver t;
  TEST_ASSERT_EQUAL_UINT16(0, t.beginHz(1000.0f));
  t.stop();

  Timer1PWM p;
  TEST_ASSERT_TRUE(p.begin(500.0f));
  p.setDuty(0, -1.0f);
  p.setDuty(1, 120.0f);
  p.stop();
}

int main(int argc, char** argv) {
  UNITY_BEGIN();

  RUN_TEST(test_analog_sampler_branches);
  RUN_TEST(test_digiin_branches);
  RUN_TEST(test_encoder_branches);
  RUN_TEST(test_cmdline_commands);
  RUN_TEST(test_timer_and_pwm_stubs);

  return UNITY_END();
}
