#include "test_support.h"

#include <string>

volatile uint8_t gTimerCallbackCountA = 0;
volatile uint8_t gTimerCallbackCountB = 0;
volatile uint8_t gTimerCallbackCountC = 0;
volatile uint8_t gTimerCallbackCountD = 0;
volatile uint8_t gTimerCallbackCountE = 0;

void timerCallbackA() {
  ++gTimerCallbackCountA;
}

void timerCallbackB() {
  ++gTimerCallbackCountB;
}

void timerCallbackC() {
  ++gTimerCallbackCountC;
}

void timerCallbackD() {
  ++gTimerCallbackCountD;
}

void timerCallbackE() {
  ++gTimerCallbackCountE;
}

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

void resetTestState() {
  mockMillis = 0;
  for (int i = 0; i < 64; ++i) mockPinModes[i] = 0xFF;
  for (int i = 0; i < 16; ++i) mockAnalogValues[i] = 0;
  mockNullInputPort = -1;
  mockNullOutputPort = -1;
  mockZeroMaskPin = -1;
  gTimerCallbackCountA = 0;
  gTimerCallbackCountB = 0;
  gTimerCallbackCountC = 0;
  gTimerCallbackCountD = 0;
  gTimerCallbackCountE = 0;
  clearPorts();
  Serial.clearOutput();
  Serial.setInput("");
}

void runCmd(FirmwareCli& cli, const char* cmd) {
  Serial.clearOutput();
  Serial.setInput(std::string(cmd) + "\n");
  cli.processSerial();
}