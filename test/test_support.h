#ifndef TEST_TEST_SUPPORT_H
#define TEST_TEST_SUPPORT_H

#include <cstdint>

#include "Arduino.h"
#include "firmware_cli.h"

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
void runCmd(FirmwareCli& cli, const char* cmd);

void test_analog_sampler_branches();
void test_avr_timer1_pwm_stubs();
void test_avr_timer2_driver_stubs();
void test_digital_input_monitor_branches();
void test_encoder_generator_branches();
void test_firmware_cli_commands();

#endif