#include <cstring>
#include <string>

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
  TEST_ASSERT_NOT_NULL(strstr(Serial.getOutput().c_str(), ":0.000"));

  runCmd(cli, "digital?");
  TEST_ASSERT_NOT_NULL(strstr(Serial.getOutput().c_str(), "\"frameSeq\":"));
  TEST_ASSERT_NOT_NULL(strstr(Serial.getOutput().c_str(), "\"stale\":"));
  TEST_ASSERT_NOT_NULL(strstr(Serial.getOutput().c_str(), "\"overrunTicks\":"));
  TEST_ASSERT_NOT_NULL(strstr(Serial.getOutput().c_str(), "\"d2\""));

  setDigitalPin(2, false);
  digitalMonitor.onTick();
  digitalMonitor.onTick();
  digitalMonitor.onTick();
  digitalMonitor.onTick();
  digitalMonitor.onTick();
  digitalMonitor.updateIfReady();
  runCmd(cli, "digital?");
  TEST_ASSERT_NOT_NULL(strstr(Serial.getOutput().c_str(), "\"stale\":true"));

  runCmd(cli, "encoder?");
  TEST_ASSERT_NOT_NULL(strstr(Serial.getOutput().c_str(), "encoder"));

  runCmd(cli, "all?");
  TEST_ASSERT_NOT_NULL(strstr(Serial.getOutput().c_str(), "\"a0\""));
  TEST_ASSERT_NOT_NULL(strstr(Serial.getOutput().c_str(), "\"frameSeq\""));
  TEST_ASSERT_NOT_NULL(strstr(Serial.getOutput().c_str(), "\"encoder\""));

  runCmd(cli, "reset");
  TEST_ASSERT_NOT_NULL(strstr(Serial.getOutput().c_str(), "resetting"));

  runCmd(cli, "pwm-freq");
  TEST_ASSERT_NOT_NULL(strstr(Serial.getOutput().c_str(), "missing frequency"));

  runCmd(cli, "pwm-freq abc");
  TEST_ASSERT_NOT_NULL(strstr(Serial.getOutput().c_str(), "invalid frequency"));

  runCmd(cli, "pwm-freq +1");
  TEST_ASSERT_NOT_NULL(strstr(Serial.getOutput().c_str(), "invalid frequency"));

  runCmd(cli, "pwm-freq 12x");
  TEST_ASSERT_NOT_NULL(strstr(Serial.getOutput().c_str(), "invalid frequency"));

  runCmd(cli, "pwm-freq 1.2345");
  TEST_ASSERT_NOT_NULL(strstr(Serial.getOutput().c_str(), "invalid frequency"));

  runCmd(cli, "pwm-freq 1.2.3");
  TEST_ASSERT_NOT_NULL(strstr(Serial.getOutput().c_str(), "invalid frequency"));

  runCmd(cli, "pwm-freq 10000000");
  TEST_ASSERT_NOT_NULL(strstr(Serial.getOutput().c_str(), "invalid frequency"));

  runCmd(cli, "pwm-freq 2147484.000");
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

  runCmd(cli, "pwm-duty 1x 50");
  TEST_ASSERT_NOT_NULL(strstr(Serial.getOutput().c_str(), "invalid channel"));

  runCmd(cli, "pwm-duty 3 50");
  TEST_ASSERT_NOT_NULL(strstr(Serial.getOutput().c_str(), "invalid channel"));

  runCmd(cli, "pwm-duty 1 abc");
  TEST_ASSERT_NOT_NULL(strstr(Serial.getOutput().c_str(), "invalid duty"));

  runCmd(cli, "pwm-duty 1 -");
  TEST_ASSERT_NOT_NULL(strstr(Serial.getOutput().c_str(), "invalid duty"));

  runCmd(cli, "pwm-duty 1 .");
  TEST_ASSERT_NOT_NULL(strstr(Serial.getOutput().c_str(), "invalid duty"));

  runCmd(cli, "pwm-duty 1 +12.5");
  TEST_ASSERT_NOT_NULL(strstr(Serial.getOutput().c_str(), "status"));

  runCmd(cli, "PWM-DUTY 0 50");
  TEST_ASSERT_NOT_NULL(strstr(Serial.getOutput().c_str(), "status"));

  runCmd(cli, "pwm-duty -1 10");
  TEST_ASSERT_NOT_NULL(strstr(Serial.getOutput().c_str(), "invalid channel"));

  runCmd(cli, "pwm-duty 1 -12.5");
  TEST_ASSERT_NOT_NULL(strstr(Serial.getOutput().c_str(), "status"));

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

void test_firmware_cli_edge_cases() {
  AnalogSampler analog;
  DigitalInputMonitor digitalMonitor;
  EncoderGenerator encoder;
  Timer1PWM pwm;

  const uint8_t aPins[] = {0, 1};
  const uint8_t dPins[] = {2, 3};
  TEST_ASSERT_TRUE(analog.begin(AnalogSampler::Config{aPins, 2, 5.0f}));
  TEST_ASSERT_TRUE(
      digitalMonitor.begin(DigitalInputMonitor::Config{dPins, 2, 2, 1000.0f, false}));
  TEST_ASSERT_TRUE(encoder.begin(EncoderGenerator::Config{9, 10, 2, 3, false, true}));

  mockAnalogValues[0] = 256;
  mockAnalogValues[1] = 768;
  analog.onTick();
  analog.sampleIfDue();

  setDigitalPin(2, false);
  setDigitalPin(3, true);
  digitalMonitor.onTick();
  setDigitalPin(2, true);
  setDigitalPin(3, true);
  digitalMonitor.onTick();
  digitalMonitor.onTick();
  digitalMonitor.updateIfReady();

  FirmwareCli cli(analog, digitalMonitor, encoder, pwm, FirmwareCli::Config{aPins, 2, dPins, 2});

  Serial.clearOutput();
  Serial.setInput("   \n");
  cli.processSerial();
  TEST_ASSERT_EQUAL_STRING("", Serial.getOutput().c_str());

  Serial.clearOutput();
  Serial.setInput("\n");
  cli.processSerial();
  TEST_ASSERT_EQUAL_STRING("", Serial.getOutput().c_str());

  Serial.clearOutput();
  Serial.setInput("analog?\r");
  cli.processSerial();
  TEST_ASSERT_NOT_NULL(strstr(Serial.getOutput().c_str(), "\"a0\""));
  TEST_ASSERT_NOT_NULL(strstr(Serial.getOutput().c_str(), ",\"a1\""));
  TEST_ASSERT_NOT_NULL(strstr(Serial.getOutput().c_str(), "\"a0\":1.251"));
  TEST_ASSERT_NOT_NULL(strstr(Serial.getOutput().c_str(), "\"a1\":3.754"));

  runCmd(cli, "digital?");
  TEST_ASSERT_NOT_NULL(strstr(Serial.getOutput().c_str(), "\"frameSeq\":1"));
  TEST_ASSERT_NOT_NULL(strstr(Serial.getOutput().c_str(), "\"stale\":true"));
  TEST_ASSERT_NOT_NULL(strstr(Serial.getOutput().c_str(), "\"overrunTicks\":1"));
  TEST_ASSERT_NOT_NULL(strstr(Serial.getOutput().c_str(), "\"d2\""));
  TEST_ASSERT_NOT_NULL(strstr(Serial.getOutput().c_str(), ",\"d3\""));

  runCmd(cli, "all?");
  TEST_ASSERT_NOT_NULL(strstr(Serial.getOutput().c_str(), "\"a0\":1.251"));
  TEST_ASSERT_NOT_NULL(strstr(Serial.getOutput().c_str(), "\"a1\":3.754"));
  TEST_ASSERT_NOT_NULL(strstr(Serial.getOutput().c_str(), "\"frameSeq\":1"));
  TEST_ASSERT_NOT_NULL(strstr(Serial.getOutput().c_str(), "\"encoder\""));

  setDigitalPin(2, false);
  setDigitalPin(3, true);
  encoder.onTick();
  runCmd(cli, "encoder?");
  TEST_ASSERT_NOT_NULL(strstr(Serial.getOutput().c_str(), "DOWN"));

  runCmd(cli, "pwm-duty 0 12.5 extra extra");
  TEST_ASSERT_NOT_NULL(strstr(Serial.getOutput().c_str(), "status"));

  Serial.clearOutput();
  Serial.setInput("abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefgh\n");
  cli.processSerial();
  TEST_ASSERT_NOT_NULL(strstr(Serial.getOutput().c_str(), "unknown command"));
}

void test_firmware_cli_internal_edges() {
  AnalogSampler analog;
  DigitalInputMonitor digitalMonitor;
  EncoderGenerator encoder;
  Timer1PWM pwm;

  const uint8_t aPins[] = {0};
  const uint8_t dPins[] = {2};
  TEST_ASSERT_TRUE(analog.begin(AnalogSampler::Config{aPins, 1, 5.0f}));
  TEST_ASSERT_TRUE(
      digitalMonitor.begin(DigitalInputMonitor::Config{dPins, 1, 2, 1000.0f, false}));
  TEST_ASSERT_TRUE(encoder.begin(EncoderGenerator::Config{9, 10, 2, 3, false, true}));

  FirmwareCli cli(analog, digitalMonitor, encoder, pwm, aPins, 1, dPins, 1);

  Serial.clearOutput();
  Serial.setInput("\r\n");
  cli.processSerial();
  TEST_ASSERT_EQUAL_STRING("", Serial.getOutput().c_str());

  Serial.setInput("   \r\n");
  cli.processSerial();
  TEST_ASSERT_EQUAL_STRING("", Serial.getOutput().c_str());

  Serial.setInput("x");
  cli.processSerial();
  TEST_ASSERT_EQUAL_STRING("", Serial.getOutput().c_str());
  Serial.setInput("\n");
  cli.processSerial();
  TEST_ASSERT_NOT_NULL(strstr(Serial.getOutput().c_str(), "unknown command"));

  runCmd(cli, "   help   extra");
  TEST_ASSERT_NOT_NULL(strstr(Serial.getOutput().c_str(), "help"));

  runCmd(cli, "RESET extra");
  TEST_ASSERT_NOT_NULL(strstr(Serial.getOutput().c_str(), "resetting"));

  Serial.clearOutput();
  advanceMillis(1);
  Serial.setInput("pwm-freq 500");
  cli.processSerial();
  TEST_ASSERT_EQUAL_STRING("", Serial.getOutput().c_str());
  advanceMillis(10);
  cli.processSerial();
  TEST_ASSERT_EQUAL_STRING("", Serial.getOutput().c_str());
  advanceMillis(100);
  cli.processSerial();
  TEST_ASSERT_NOT_NULL(strstr(Serial.getOutput().c_str(), "status"));

  Serial.clearOutput();
  std::string longUnknown(96, 'z');
  Serial.setInput(longUnknown + "\n");
  cli.processSerial();
  TEST_ASSERT_NOT_NULL(strstr(Serial.getOutput().c_str(), "unknown command"));
}