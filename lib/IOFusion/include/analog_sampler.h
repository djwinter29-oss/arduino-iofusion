/// @file analog_sampler.h
/// @brief Analog input sampler for Arduino AVR targets.
#ifndef IOFUSION_ANALOG_SAMPLER_H
#define IOFUSION_ANALOG_SAMPLER_H

#include <Arduino.h>

/// @brief Samples one or more analog channels on loop-side demand.
///
/// The ISR-facing API only sets a pending flag via onTick(). The actual ADC reads
/// are deferred to sampleIfDue() so interrupt latency stays predictable.
class AnalogSampler {
 public:
  /// @brief Startup configuration for AnalogSampler.
  struct Config {
    /// Analog channel list, typically values in the range 0..5 on Uno-class boards.
    const uint8_t* channels = nullptr;
    /// Number of entries in @ref channels.
    uint8_t channelCount = 0;
    /// Reference voltage used when scaling raw ADC readings to volts.
    float vref = 5.0f;

    Config() = default;
    Config(const uint8_t* channelsIn, uint8_t channelCountIn, float vrefIn)
      : channels(channelsIn), channelCount(channelCountIn), vref(vrefIn) {}
  };

  /// @brief Constructs a sampler with no configured channels.
  AnalogSampler();

  /// @brief Configures the sampler from a typed configuration object.
  /// @param config Channel list and scaling configuration.
  /// @return `true` when the configuration is valid for the current target.
  bool begin(const Config& config);

  /// @brief Convenience overload that forwards to @ref begin(const Config&).
  /// @param channels Pointer to a list of analog channels.
  /// @param count Number of channels in @p channels.
  /// @return `true` when the configuration is valid.
  bool begin(const uint8_t* channels, uint8_t count);

  /// @brief Requests one sampling round from ISR context.
  void onTick();

  /// @brief Performs pending ADC reads from loop context.
  void sampleIfDue();

  /// @brief Returns the number of configured analog channels.
  uint8_t getChannelCount() const;

  /// @brief Returns the most recent scaled channel voltage.
  /// @param idx Logical channel index within the configured channel list.
  /// @return Voltage in the range 0.0 .. Vref, or 0.0 when @p idx is out of range.
  float getValue(uint8_t idx) const;

  /// @brief Updates the voltage reference used for scaling ADC readings.
  /// @param vref New reference voltage in volts.
  void setVref(float vref);

 private:
  static const uint8_t MAX_CHANNELS = 6;
  uint8_t _channels[MAX_CHANNELS];
  uint8_t _channelCount = 0;
  volatile bool _sampleRequested = false;
  int _lastValues[MAX_CHANNELS];
  float _vref = 5.0f;
};

#endif  // IOFUSION_ANALOG_SAMPLER_H
