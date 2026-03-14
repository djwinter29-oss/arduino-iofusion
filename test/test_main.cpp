#include <unity.h>

#include "test_support.h"

void setUp() {
  resetTestState();
}

void tearDown() {}

int main(int argc, char** argv) {
  UNITY_BEGIN();

  RUN_TEST(test_analog_sampler_branches);
  RUN_TEST(test_analog_sampler_config_edges);
  RUN_TEST(test_digital_input_monitor_branches);
  RUN_TEST(test_digital_input_monitor_config_edges);
  RUN_TEST(test_encoder_generator_branches);
  RUN_TEST(test_encoder_generator_config_edges);
  RUN_TEST(test_firmware_cli_commands);
  RUN_TEST(test_firmware_cli_edge_cases);
  RUN_TEST(test_firmware_cli_internal_edges);

  return UNITY_END();
}