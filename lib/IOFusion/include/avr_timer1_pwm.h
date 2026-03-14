/// @file avr_timer1_pwm.h
/// @brief AVR Timer1 PWM driver for Uno-class Arduino targets.
#ifndef IOFUSION_AVR_TIMER1_PWM_H
#define IOFUSION_AVR_TIMER1_PWM_H

#include <Arduino.h>

/// @brief Controls the two hardware PWM outputs driven by AVR Timer1.
class Timer1PWM {
 public:
  /// @brief Startup configuration for Timer1PWM.
  struct Config {
    /// Requested PWM frequency in hertz.
    float frequencyHz = 0.0f;

    Config() = default;
    explicit Config(float frequencyHzIn) : frequencyHz(frequencyHzIn) {}
  };

  /// @brief Constructs a stopped PWM controller.
  Timer1PWM();

  /// @brief Configures Timer1 from a typed configuration object.
  /// @param config Requested PWM frequency.
  /// @return `true` when the requested frequency can be represented on Timer1.
  bool begin(const Config& config);

  /// @brief Convenience overload that forwards to @ref begin(const Config&).
  bool begin(float freqHz);

  /// @brief Updates the duty cycle for a hardware PWM channel.
  /// @param channel Hardware channel index: 0 for OC1A, 1 for OC1B.
  /// @param percent Duty cycle percentage. Values are clamped to 0..100.
  void setDuty(uint8_t channel, float percent);

  /// @brief Stops PWM generation and releases the hardware pins.
  void stop();

 private:
  uint16_t _top = 0;       // ICR1 top value
  uint16_t _presBits = 0;  // CS bits in TCCR1B
  float _dutyPercent[2] = {0.0f, 0.0f};
  bool _configured = false;
  uint16_t percentToCounts(float percent, uint16_t top) const;
  void _applyDuty(uint8_t channel, float percent, uint16_t top);
};

#endif  // IOFUSION_AVR_TIMER1_PWM_H
