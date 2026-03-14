#ifndef FIRMWARE_CLI_H
#define FIRMWARE_CLI_H

#include <Arduino.h>

#include "analog_sampler.h"
#include "avr_timer1_pwm.h"
#include "digital_input_monitor.h"
#include "encoder_generator.h"

class FirmwareCli {
 public:
  struct Config {
    const uint8_t* analogPins = nullptr;
    uint8_t analogCount = 0;
    const uint8_t* digitalPins = nullptr;
    uint8_t digitalCount = 0;

    Config() = default;
    Config(const uint8_t* analogPinsIn, uint8_t analogCountIn, const uint8_t* digitalPinsIn,
           uint8_t digitalCountIn)
        : analogPins(analogPinsIn),
          analogCount(analogCountIn),
          digitalPins(digitalPinsIn),
          digitalCount(digitalCountIn) {}
  };

  FirmwareCli(AnalogSampler& analog, DigitalInputMonitor& digitalMonitor, EncoderGenerator& encoder,
              Timer1PWM& pwm, const Config& config);
  FirmwareCli(AnalogSampler& analog, DigitalInputMonitor& digitalMonitor, EncoderGenerator& encoder,
              Timer1PWM& pwm, const uint8_t* analogPins, uint8_t analogCount,
              const uint8_t* digitalPins, uint8_t digitalCount);

  void processSerial();

 private:
  void appendAnalogFields(bool& firstField);
  void appendDigitalFields(bool& firstField, const DigitalInputMonitor::Frame& frame);
  void appendEncoderFields(bool& firstField);
  void respondAnalog();
  void respondDigital();
  void respondEncoder();
  void respondAll();
  void resetBoard();
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