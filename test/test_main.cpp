#include <unity.h>

#include "test_support.h"

void setUp() {
  resetTestState();
}

void tearDown() {}

int main(int argc, char** argv) {
  UNITY_BEGIN();

  RUN_TEST(test_analog_sampler_branches);
  RUN_TEST(test_digiin_branches);
  RUN_TEST(test_encoder_branches);
  RUN_TEST(test_cmdline_commands);
  RUN_TEST(test_timer_and_pwm_stubs);

  return UNITY_END();
}