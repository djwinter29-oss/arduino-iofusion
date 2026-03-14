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
void test_analog_sampler_config_edges();
void test_digital_input_monitor_branches();
void test_digital_input_monitor_config_edges();
void test_encoder_generator_branches();
void test_encoder_generator_config_edges();
void test_encoder_generator_position_saturates();
void test_firmware_cli_commands();
void test_firmware_cli_edge_cases();
void test_firmware_cli_internal_edges();

#endif