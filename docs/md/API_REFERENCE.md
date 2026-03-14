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
- `begin()` is a startup-time configuration API across the library. In the intended Arduino/Uno use case, components are configured once during board startup and then used with a steady runtime contract rather than repeatedly reconfigured during normal operation.
- Pin maps and timer ownership are caller-managed. To keep the library lightweight on Uno-class targets, IOFusion does not attempt exhaustive runtime detection of overlapping pin assignments or cross-module timer conflicts.

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
  - Intended to be called during board startup before normal runtime sampling begins.
  - Returns `false` if the channel list is invalid or `config.vref` cannot be represented as a positive millivolt value.
- `bool begin(const uint8_t* channels, uint8_t count)`
  - Convenience overload for channel-only setup.
  - Same startup-only intent as the typed `Config` overload.
  - Returns `false` if `count == 0` or `count > 6`.

- `void onTick()`
  - ISR-side trigger: requests one sampling round.
  - This is a best-effort request, not a guaranteed per-tick conversion contract.
  - If `sampleIfDue()` has not yet drained the previous request, repeated `onTick()` calls coalesce into one pending sampling round.

- `void sampleIfDue()`
  - Loop-side execution: reads ADC for configured channels when requested.

- `uint8_t getChannelCount() const`
- `float getValue(uint8_t idx) const`
  - Returns voltage in volts using configured `Vref`.
  - Out-of-range `idx` returns `0.0`.
- `uint16_t getMillivolts(uint8_t idx) const`
  - Returns the scaled analog value in millivolts.

- `void setVref(float vref)`
  - Sets ADC scaling reference voltage.
  - Non-positive values are ignored.
- `void setVrefMillivolts(uint16_t vrefMillivolts)`
  - Fixed-point alternative for AVR-friendly callers.

Implementation note:

- `AnalogSampler` stores `Vref` internally in millivolts and converts to float only in `getValue()`.

---

## `DigitalInputMonitor`

Header: `lib/IOFusion/include/digital_input_monitor.h`

Preferred setup:

- `struct DigitalInputMonitor::Config { const uint8_t* pins; uint8_t pinCount; uint16_t windowTicks; float tickHz; bool usePullup; }`

### Methods

- `bool begin(const Config& config)`
  - Preferred setup entry point.
  - Intended to be called during board startup before the Timer2-driven sampling loop is active.
- `bool begin(const uint8_t* pins, uint8_t count, uint16_t windowTicks=1000, float tickHz=1000.0f, bool usePullup=false)`
  - Convenience overload for positional setup.
  - Same startup-only intent as the typed `Config` overload.
  - Returns `false` on invalid arguments or pin mapping failure.
  - `DigitalInputMonitor` is a sampled estimator: pulses shorter than one tick may be missed, and input frequencies above $\frac{\text{tickHz}}{2}$ alias.
  - Frequency resolution is $\frac{\text{tickHz}}{\text{windowTicks}}$ Hz.
  - Duty resolution is approximately $\frac{100}{\text{windowTicks}}\%$.
  - For robust frequency and duty measurements, keep the input comfortably below Nyquist; a practical target is $f_{in} \le \frac{\text{tickHz}}{4}$.

- `void onTick()`
  - ISR-side sampling and counter accumulation.
  - If the previous sampling window has not yet been drained, the monitor increments an overrun counter and marks the next published frame stale instead of silently sampling stale data.

- `void updateIfReady()`
  - Loop-side conversion to frequency (Hz) and duty (%).

- `uint8_t getPinCount() const`
- `void copyFrame(Frame& frame) const`
  - Copies the currently published frame metadata and published per-pin results under one critical section.
  - Prefer this when a caller needs one coherent telemetry snapshot instead of field-by-field reads.
- `float getFrequency(uint8_t idx) const`
- `uint32_t getFrequencyMilliHz(uint8_t idx) const`
- `float getDutyCycle(uint8_t idx) const`
- `uint16_t getDutyPermille(uint8_t idx) const`
  - Out-of-range `idx` returns `0.0`.
- `bool isFrameStale() const`
  - Returns `true` when the currently published measurement frame was published after one or more ticks were dropped before that frame could be drained.
  - The value is latched when `updateIfReady()` publishes a frame and remains attached to that frame until the next publish.
- `uint32_t getFrameSequence() const`
  - Returns a monotonic sequence number for the published measurement frame.
  - The value increments each time `updateIfReady()` publishes a new frame.
- `uint32_t getOverrunCount() const`
  - Returns the cumulative number of timer ticks skipped because `updateIfReady()` had not yet drained the completed window.
  - This is a monotonic fault counter, not a per-window statistic.
  - The value saturates at `UINT32_MAX` rather than wrapping.
  - A non-zero count indicates loop-side lag or serial/backpressure stalls.

Implementation note:

- `DigitalInputMonitor` stores computed results internally as fixed-point (`millihertz` and `permille`) and converts to float only in the compatibility getters.

---

## `EncoderGenerator`

Header: `lib/IOFusion/include/encoder_generator.h`

Preferred setup:

- `struct EncoderGenerator::Config { uint8_t pinA; uint8_t pinB; uint8_t upPin; uint8_t downPin; bool usePullup; bool activeHigh; }`

### Methods

- `bool begin(const Config& config)`
  - Preferred setup entry point.
  - Intended to be called during board startup before encoder generation begins.
- `bool begin(uint8_t pinA, uint8_t pinB, uint8_t up, uint8_t down, bool usePullup=false, bool activeHigh=true)`
  - Convenience overload for positional setup.
  - Same startup-only intent as the typed `Config` overload.
  - Configures quadrature outputs (`pinA`, `pinB`) and direction inputs (`up`, `down`).
  - The caller is responsible for choosing non-overlapping pins; this API does not try to validate every cross-role wiring conflict at runtime.
  - Default control semantics are logic-driven, active-HIGH inputs.
  - For direct switch-to-ground wiring, use `usePullup=true` and `activeHigh=false`.

- `void onTick()`
  - ISR-side state advance based on direction input levels.

  - `int32_t getPosition()`
    - Returns the absolute generated position count relative to startup or the most recent `reset()`.
    - The count saturates at the `int32_t` limits instead of wrapping.
- `bool getDirection()`
  - Direction semantics: `true` => UP, `false` => DOWN.

- `void reset()`
    - Resets position/state and drives outputs low.
    - Establishes a new zero origin for subsequent absolute position reads.

---

## `Timer1PWM`

Header: `lib/IOFusion/include/avr_timer1_pwm.h`

Preferred setup:

- `struct Timer1PWM::Config { float frequencyHz; }`

### Methods

- `bool begin(const Config& config)`
  - Preferred setup entry point.
  - Intended to be called during board startup before PWM output is enabled.
- `bool begin(float freqHz)`
  - Convenience overload for direct frequency setup.
  - Same startup-only intent as the typed `Config` overload.
  - The caller is responsible for ensuring Timer1 and pins D9/D10 are not already committed elsewhere in the application.

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
  - Intended to be called during board startup before the scheduler is attached to application work.
- `uint16_t beginHz(float freqHz)`
  - Convenience overload for direct frequency setup.
  - Starts Timer2 near requested frequency.
  - Timer2 configuration is startup-only; runtime retuning is intentionally not supported.
  - Returns the OCR value used on AVR, or `0` on invalid input / when Timer2 is already active.
  - Bring-up is ordered so Timer2 counter/pending flags are cleared before compare interrupts are armed.
  - Call `stop()` first if you need to release Timer2 and configure it again.
  - The caller is responsible for ensuring Timer2 is not needed by other firmware features on the target.

- `void stop()`
- `bool attachCallback(Timer2Callback cb)`
  - Returns `false` for null callbacks, duplicates, callback-table overflow, or inactive drivers.
- `bool detachCallback(Timer2Callback cb)`
  - Returns `false` when the callback is not registered on the active driver.
- `static void handleInterrupt()`

---

## Reference firmware command surface (non-library)

Source: `apps/reference_firmware/src/firmware_cli.cpp`

Intended use:

- The serial command surface is suitable both for occasional human-driven diagnostics and for low-rate host polling from a supervisory application.
- In the normal reference-firmware use case, a host application such as Python polling about once per second is an appropriate operating model.
- `digital?` is intended to serve both of those cases, which is why the reference firmware builds its response from one coherent published-frame snapshot.

Supported commands (case-insensitive command token):

- `analog?`
- `digital?`
- `encoder?`
- `all?`
- `pwm-freq <hz>`
- `pwm-duty <ch> <pct>`
- `reset`
- `help`

Response contract:

- Success: `{"status":"ok"}` for mutating PWM commands.
- `reset` returns `{"status":"resetting"}` immediately before the reference firmware requests a board reset.
- Errors (stable keys): `{"error":"..."}`.
- Unknown command: `{"error":"unknown command"}`.
- `digital?` responses include `overrunTicks` so stale sampling windows are detectable from the reference firmware.
- `digital?` responses also include `frameSeq` and `stale` so freshness is attached to the reported measurement frame itself.
- `all?` returns one combined JSON object containing analog fields, the coherent digital frame fields, and the encoder object.

Parser behavior notes:

- Leading/trailing and repeated spaces are accepted.
- Extra tokens after a known command are ignored by current implementation.

## Compatibility scope

IOFusion is intentionally maintained for **Arduino-focused** usage. Compatibility commitments in this document are made within that scope.
