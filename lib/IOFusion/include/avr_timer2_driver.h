/// @file avr_timer2_driver.h
/// @brief AVR Timer2 tick driver for Uno-class Arduino targets.
#ifndef IOFUSION_AVR_TIMER2_DRIVER_H
#define IOFUSION_AVR_TIMER2_DRIVER_H

#include <Arduino.h>

typedef void (*Timer2Callback)();

/// @brief Provides a periodic Timer2 interrupt source and callback dispatch table.
class Timer2Driver {
 public:
  /// @brief Startup configuration for Timer2Driver.
  struct Config {
    /// Requested tick frequency in hertz.
    float frequencyHz = 0.0f;

    Config() = default;
    explicit Config(float frequencyHzIn) : frequencyHz(frequencyHzIn) {}
  };

  /// @brief Constructs an inactive Timer2 driver.
  Timer2Driver();

  /// @brief Starts Timer2 from a typed configuration object.
  /// @param config Requested Timer2 frequency.
  /// @return AVR OCR value used for the configured timer, or 0 on failure.
  /// Timer2 configuration is startup-only: call @ref stop() before attempting
  /// any reconfiguration.
  uint16_t begin(const Config& config);

  /// @brief Convenience overload that forwards to @ref begin(const Config&).
  /// Returns 0 if Timer2 is already active, including repeated calls on the
  /// same driver instance.
  uint16_t beginHz(float freqHz);

  /// @brief Stops Timer2 and disables interrupt dispatch.
  void stop();

  /// @brief Registers a callback that executes from ISR context.
  bool attachCallback(Timer2Callback cb);
  /// @brief Removes a previously registered ISR callback.
  bool detachCallback(Timer2Callback cb);
  /// @brief ISR entry point used by the Timer2 compare-match vector.
  static void handleInterrupt();

 private:
  static const uint8_t MAX_CALLBACKS = 4;
  volatile Timer2Callback _cbs[MAX_CALLBACKS];
  static Timer2Driver* volatile _activeDriver;

  void resetCallbacks();
  void dispatchCallbacks();
};

#endif  // IOFUSION_AVR_TIMER2_DRIVER_H
