/// @file encoder_generator.h
/// @brief Quadrature output generator controlled by digital direction inputs.
#ifndef IOFUSION_ENCODER_GENERATOR_H
#define IOFUSION_ENCODER_GENERATOR_H

#include <Arduino.h>

/// @brief Generates quadrature A/B output transitions from up/down control signals.
class EncoderGenerator {
 public:
  /// @brief Startup configuration for EncoderGenerator.
  struct Config {
    /// Quadrature channel A output pin.
    uint8_t pinA = 255;
    /// Quadrature channel B output pin.
    uint8_t pinB = 255;
    /// Direction control input that advances the generated waveform.
    uint8_t upPin = 255;
    /// Direction control input that reverses the generated waveform.
    uint8_t downPin = 255;
    /// Enables INPUT_PULLUP on the direction inputs when true.
    bool usePullup = false;
    /// Interprets asserted direction inputs as HIGH when true, LOW when false.
    bool activeHigh = true;

    Config() = default;
    Config(uint8_t pinAIn, uint8_t pinBIn, uint8_t upPinIn, uint8_t downPinIn, bool usePullupIn,
           bool activeHighIn)
        : pinA(pinAIn),
          pinB(pinBIn),
          upPin(upPinIn),
          downPin(downPinIn),
          usePullup(usePullupIn),
          activeHigh(activeHighIn) {}
  };

  /// @brief Configures output and control pins from a typed configuration object.
  /// @param config Pin and polarity settings for the generator.
  /// @return `true` when the configuration is valid for the current target.
  bool begin(const Config& config);

  /// @brief Convenience overload that forwards to @ref begin(const Config&).
  bool begin(uint8_t pinA, uint8_t pinB, uint8_t up, uint8_t down, bool usePullup = false,
             bool activeHigh = true);

  /// @brief Advances the generated waveform by one step from ISR context.
  void onTick();

  /// @brief Returns the absolute generated position count.
  /// The count is relative to startup or the most recent @ref reset() call.
  /// It saturates at the `int32_t` limits instead of wrapping.
  int32_t getPosition();
  /// @brief Returns the last generated direction.
  bool getDirection();
  /// @brief Resets waveform state and absolute position to the idle state.
  /// This establishes a new zero origin for subsequent position reads.
  void reset();

 private:
  // Instance state
  uint8_t _pinA = 255;
  uint8_t _pinB = 255;
  uint8_t _state = 0;
  volatile uint8_t* _portAOut = nullptr;
  volatile uint8_t* _portBOut = nullptr;
  uint8_t _maskA = 0;
  uint8_t _maskB = 0;
  uint8_t _pinUp = 255;
  uint8_t _pinDown = 255;
  volatile uint8_t* _upPortIn = nullptr;
  volatile uint8_t* _downPortIn = nullptr;
  uint8_t _upMask = 0;
  uint8_t _downMask = 0;
  bool _activeHigh = true;
  volatile int32_t _position = 0;
  volatile bool _directionUp = true;
};

// No global instance here — create an instance in your `main.cpp` as needed.

#endif  // IOFUSION_ENCODER_GENERATOR_H
