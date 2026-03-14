/// @file digital_input_monitor.h
/// @brief Sampled digital input monitor for Arduino AVR targets.
#ifndef IOFUSION_DIGITAL_INPUT_MONITOR_H
#define IOFUSION_DIGITAL_INPUT_MONITOR_H

#include <Arduino.h>

/// @brief Estimates frequency and duty cycle from sampled digital inputs.
///
/// This component is intentionally a sampled estimator, not a hardware capture block.
/// Accuracy depends on the configured tick rate and measurement window.
class DigitalInputMonitor {
 public:
  /// @brief Startup configuration for DigitalInputMonitor.
  struct Config {
    /// Pointer to the input pin list.
    const uint8_t* pins = nullptr;
    /// Number of entries in @ref pins.
    uint8_t pinCount = 0;
    /// Number of timer ticks per measurement window.
    uint16_t windowTicks = 1000;
    /// Sampling tick frequency in hertz.
    float tickHz = 1000.0f;
    /// Enables INPUT_PULLUP on every monitored pin when true.
    bool usePullup = false;

    Config() = default;
    Config(const uint8_t* pinsIn, uint8_t pinCountIn, uint16_t windowTicksIn, float tickHzIn,
           bool usePullupIn)
        : pins(pinsIn),
          pinCount(pinCountIn),
          windowTicks(windowTicksIn),
          tickHz(tickHzIn),
          usePullup(usePullupIn) {}
  };

  /// @brief Constructs an unconfigured monitor.
  DigitalInputMonitor();

  /// @brief Configures monitored pins and estimator parameters.
  /// @param config Pin list and timing configuration.
  /// @return `true` when the configuration is valid for the current target.
  bool begin(const Config& config);

  /// @brief Convenience overload that forwards to @ref begin(const Config&).
  bool begin(const uint8_t* pins, uint8_t count, uint16_t windowTicks = 1000,
             float tickHz = 1000.0f, bool usePullup = false);

  /// @brief Samples the monitored inputs once from ISR context.
  void onTick();
  /// @brief Converts the most recent completed sampling window into frequency and duty estimates.
  void updateIfReady();

  /// @brief Returns the number of configured pins.
  uint8_t getPinCount() const;
  /// @brief Returns the latest frequency estimate for a configured pin.
  float getFrequency(uint8_t idx) const;
  /// @brief Returns the latest duty-cycle estimate for a configured pin.
  float getDutyCycle(uint8_t idx) const;

 private:
  static const uint8_t MAX_PINS = 8;
  uint8_t _pins[MAX_PINS];
  uint8_t _pinCount = 0;
  uint16_t _windowTicks = 1000;
  float _tickHz = 1000.0f;
  volatile uint8_t* _pinPortIn[MAX_PINS];
  uint8_t _pinMask[MAX_PINS];
  volatile uint16_t _samplesInWindow = 0;
  volatile uint16_t _edgeCnt[MAX_PINS];
  volatile uint16_t _highCnt[MAX_PINS];
  volatile uint8_t _lastState[MAX_PINS];
  volatile bool _windowReady = false;
  float _freq[MAX_PINS];
  float _duty[MAX_PINS];
};

#endif  // IOFUSION_DIGITAL_INPUT_MONITOR_H