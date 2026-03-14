#include <cstring>

#include <unity.h>

#include "analog_sampler.h"
#include "avr_timer1_pwm.h"
#include "digital_input_monitor.h"
#include "encoder_generator.h"
#include "firmware_cli.h"
#include "test_support.h"

void test_firmware_cli_commands() {
  AnalogSampler analog;
  DigitalInputMonitor digitalMonitor;
  EncoderGenerator encoder;
  Timer1PWM pwm;

  const uint8_t aPins[] = {0};
  const uint8_t dPins[] = {2};
  TEST_ASSERT_TRUE(analog.begin(AnalogSampler::Config{aPins, 1, 5.0f}));
  TEST_ASSERT_TRUE(
      digitalMonitor.begin(DigitalInputMonitor::Config{dPins, 1, 4, 1000.0f, false}));
  TEST_ASSERT_TRUE(encoder.begin(EncoderGenerator::Config{9, 10, 2, 3, false, true}));

  FirmwareCli cli(analog, digitalMonitor, encoder, pwm, FirmwareCli::Config{aPins, 1, dPins, 1});

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

  runCmd(cli, "  PWM-FREQ   500   ");
  TEST_ASSERT_NOT_NULL(strstr(Serial.getOutput().c_str(), "status"));

  runCmd(cli, "pwm-freq 0");
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

  runCmd(cli, "PWM-DUTY 0 50");
  TEST_ASSERT_NOT_NULL(strstr(Serial.getOutput().c_str(), "status"));

  runCmd(cli, "pwm-duty -1 10");
  TEST_ASSERT_NOT_NULL(strstr(Serial.getOutput().c_str(), "invalid channel"));

  runCmd(cli, "pwm-duty 1 12.5x");
  TEST_ASSERT_NOT_NULL(strstr(Serial.getOutput().c_str(), "invalid duty"));

  runCmd(cli, "pwm-duty 1 12.5");
  TEST_ASSERT_NOT_NULL(strstr(Serial.getOutput().c_str(), "status"));

  runCmd(cli, "unknown-cmd");
  TEST_ASSERT_NOT_NULL(strstr(Serial.getOutput().c_str(), "unknown command"));

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