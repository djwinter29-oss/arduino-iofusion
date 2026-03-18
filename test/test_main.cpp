#include <unity.h>

#include "test_support.h"

void test_digital_out_begin_rejects_invalid_args();
void test_digital_out_begin_and_basic_ops();
void test_digital_out_index_bounds();

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
  RUN_TEST(test_digital_input_monitor_copy_frame);
  RUN_TEST(test_encoder_generator_branches);
  RUN_TEST(test_encoder_generator_config_edges);
  RUN_TEST(test_encoder_generator_position_saturates);
  RUN_TEST(test_firmware_cli_commands);
  RUN_TEST(test_firmware_cli_edge_cases);
  RUN_TEST(test_firmware_cli_internal_edges);
  RUN_TEST(test_digital_out_begin_rejects_invalid_args);
  RUN_TEST(test_digital_out_begin_and_basic_ops);
  RUN_TEST(test_digital_out_index_bounds);

  return UNITY_END();
}