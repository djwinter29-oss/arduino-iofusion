# API Reference

This document describes the current public API contract and stability policy for IOFusion.

## Scope Statement

- Target: Arduino ecosystem, primarily Uno-class targets.
- Policy: keep the API simple and stable for Arduino use cases.
- Non-goal: generic cross-platform HAL abstraction in the current phase.

## General Behavioral Contract

- ISR-facing entry points such as `onTick()` must remain minimal.
- Loop-facing methods perform heavier math, formatting, and deferred work.
- Call update methods regularly from `loop()`; the library is not designed around long blocking sections.

## Stability Policy

### Stable API

Public headers under `lib/IOFusion/include/` are considered stable unless documented otherwise.

- Backward compatibility is expected across patch and minor releases.
- Breaking changes should be reserved for a major version bump.

### Internal API

Implementation files under `lib/IOFusion/src/` and the reference firmware under `apps/reference_firmware/` are internal.

- They may change without notice.
- They are not the intended integration surface for downstream consumers.

### Experimental API

Any API explicitly marked experimental in docs or release notes may change between minor releases.

### Versioning Rules

- PATCH (`x.y.Z`): bug fixes, tests, docs, no intended public API break.
- MINOR (`x.Y.z`): backward-compatible additions.
- MAJOR (`X.y.z`): breaking API changes.

### Deprecation Policy

- Deprecations should be called out in release notes or docs before removal.
- Prefer at least one minor release of deprecation runway before deleting stable surface area.

## `AnalogSampler`

Header: `lib/IOFusion/include/analog_sampler.h`

Preferred setup:

- `struct AnalogSampler::Config { const uint8_t* channels; uint8_t channelCount; float vref; }`

### Methods

- `bool begin(const Config& config)`
  - Preferred setup entry point.
  - Applies both channel mapping and voltage-reference scaling.
- `bool begin(const uint8_t* channels, uint8_t count)`
  - Convenience overload for channel-only setup.
  - Returns `false` if `count == 0` or `count > 6`.

- `void onTick()`
  - ISR-side trigger: requests one sampling round.

- `void sampleIfDue()`
  - Loop-side execution: reads ADC for configured channels when requested.

- `uint8_t getChannelCount() const`
- `float getValue(uint8_t idx) const`
  - Returns voltage in volts using configured `Vref`.
  - Out-of-range `idx` returns `0.0`.

- `void setVref(float vref)`
  - Sets ADC scaling reference voltage.
  - Non-positive values are ignored.

---

## `DigitalInputMonitor`

Header: `lib/IOFusion/include/digital_input_monitor.h`

Preferred setup:

- `struct DigitalInputMonitor::Config { const uint8_t* pins; uint8_t pinCount; uint16_t windowTicks; float tickHz; bool usePullup; }`

### Methods

- `bool begin(const Config& config)`
  - Preferred setup entry point.
- `bool begin(const uint8_t* pins, uint8_t count, uint16_t windowTicks=1000, float tickHz=1000.0f, bool usePullup=false)`
  - Convenience overload for positional setup.
  - Returns `false` on invalid arguments or pin mapping failure.
  - `DigitalInputMonitor` is a sampled estimator: pulses shorter than one tick may be missed, and input frequencies above $\frac{\text{tickHz}}{2}$ alias.
  - Frequency resolution is $\frac{\text{tickHz}}{\text{windowTicks}}$ Hz.
  - Duty resolution is approximately $\frac{100}{\text{windowTicks}}\%$.
  - For robust frequency and duty measurements, keep the input comfortably below Nyquist; a practical target is $f_{in} \le \frac{\text{tickHz}}{4}$.

- `void onTick()`
  - ISR-side sampling and counter accumulation.

- `void updateIfReady()`
  - Loop-side conversion to frequency (Hz) and duty (%).

- `uint8_t getPinCount() const`
- `float getFrequency(uint8_t idx) const`
- `float getDutyCycle(uint8_t idx) const`
  - Out-of-range `idx` returns `0.0`.

---

## `EncoderGenerator`

Header: `lib/IOFusion/include/encoder_generator.h`

Preferred setup:

- `struct EncoderGenerator::Config { uint8_t pinA; uint8_t pinB; uint8_t upPin; uint8_t downPin; bool usePullup; bool activeHigh; }`

### Methods

- `bool begin(const Config& config)`
  - Preferred setup entry point.
- `bool begin(uint8_t pinA, uint8_t pinB, uint8_t up, uint8_t down, bool usePullup=false, bool activeHigh=true)`
  - Convenience overload for positional setup.
  - Configures quadrature outputs (`pinA`, `pinB`) and direction inputs (`up`, `down`).
  - Default control semantics are logic-driven, active-HIGH inputs.
  - For direct switch-to-ground wiring, use `usePullup=true` and `activeHigh=false`.

- `void onTick()`
  - ISR-side state advance based on direction input levels.

- `int32_t getPosition()`
- `bool getDirection()`
  - Direction semantics: `true` => UP, `false` => DOWN.

- `void reset()`
  - Resets position/state and drives outputs low.

---

## `Timer1PWM`

Header: `lib/IOFusion/include/avr_timer1_pwm.h`

Preferred setup:

- `struct Timer1PWM::Config { float frequencyHz; }`

### Methods

- `bool begin(const Config& config)`
  - Preferred setup entry point.
- `bool begin(float freqHz)`
  - Convenience overload for direct frequency setup.

- `void setDuty(uint8_t channel, float percent)`
  - Channel `0`/`1`, duty in `0..100` (input is clamped).
  - `0%` drives a steady LOW output level.
  - `100%` drives a steady HIGH output level.

- `void stop()`
  - Disables PWM outputs and clears setup state.

---

## `Timer2Driver`

Header: `lib/IOFusion/include/avr_timer2_driver.h`

Preferred setup:

- `struct Timer2Driver::Config { float frequencyHz; }`

### Methods

- `uint16_t begin(const Config& config)`
  - Preferred setup entry point.
- `uint16_t beginHz(float freqHz)`
  - Convenience overload for direct frequency setup.
  - Starts Timer2 near requested frequency.
  - Returns the OCR value used on AVR, or `0` on invalid input / when Timer2 is already owned by another `Timer2Driver` instance.
  - Clears any previously registered callbacks for that driver instance.

- `void stop()`
- `bool attachCallback(Timer2Callback cb)`
  - Returns `false` for null callbacks, duplicates, callback-table overflow, or inactive drivers.
- `bool detachCallback(Timer2Callback cb)`
  - Returns `false` when the callback is not registered on the active driver.
- `static void handleInterrupt()`

---

## Reference firmware command surface (non-library)

Source: `apps/reference_firmware/src/firmware_cli.cpp`

Supported commands (case-insensitive command token):

- `analog?`
- `digital?`
- `encoder?`
- `pwm-freq <hz>`
- `pwm-duty <ch> <pct>`
- `help`

Response contract:

- Success: `{"status":"ok"}` for mutating PWM commands.
- Errors (stable keys): `{"error":"..."}`.
- Unknown command: `{"error":"unknown command"}`.

Parser behavior notes:

- Leading/trailing and repeated spaces are accepted.
- Extra tokens after a known command are ignored by current implementation.

## Compatibility scope

IOFusion is intentionally maintained for **Arduino-focused** usage. Compatibility commitments in this document are made within that scope.
