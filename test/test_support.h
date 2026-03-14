#ifndef TEST_TEST_SUPPORT_H
#define TEST_TEST_SUPPORT_H

#include <cstdint>

#include "Arduino.h"
#include "cmdline.h"

extern volatile uint8_t gTimerCallbackCountA;
extern volatile uint8_t gTimerCallbackCountB;
extern volatile uint8_t gTimerCallbackCountC;
extern volatile uint8_t gTimerCallbackCountD;
extern volatile uint8_t gTimerCallbackCountE;

void timerCallbackA();
void timerCallbackB();
void timerCallbackC();
void timerCallbackD();
void timerCallbackE();

void setDigitalPin(uint8_t pin, bool high);
void clearPorts();
void resetTestState();
void runCmd(CmdLine& cli, const char* cmd);

void test_analog_sampler_branches();
void test_digiin_branches();
void test_encoder_branches();
void test_cmdline_commands();
void test_timer_and_pwm_stubs();

#endif