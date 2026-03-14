#ifndef FIRMWARE_CLI_H
#define FIRMWARE_CLI_H

#include <Arduino.h>

#include "analog_sampler.h"
#include "digital_input_monitor.h"
#include "encoder_generator.h"
#include "avr_timer1_pwm.h"

class FirmwareCli {
 public:
  FirmwareCli(AnalogSampler& analog, DigitalInputMonitor& digitalMonitor,
              EncoderGenerator& encoder, Timer1PWM& pwm, const uint8_t* analogPins,
              uint8_t analogCount, const uint8_t* digitalPins, uint8_t digitalCount);

  void processSerial();

 private:
  void respondAnalog();
  void respondDigital();
  void respondEncoder();
  void handleCommand(char* cmd);
  void dispatchCommand();

  AnalogSampler& _analog;
  DigitalInputMonitor& _digitalMonitor;
  EncoderGenerator& _encoder;
  Timer1PWM& _pwm;
  const uint8_t* _analogPins;
  uint8_t _analogCount;
  const uint8_t* _digitalPins;
  uint8_t _digitalCount;

  static constexpr size_t kCmdBufferSize = 64;
  static constexpr unsigned long kCmdIdleTimeoutMs = 75;
  char _cmdBuffer[kCmdBufferSize] = {0};
  size_t _cmdLength = 0;
  unsigned long _lastByteTimeMs = 0;
};

#endif  // FIRMWARE_CLI_H