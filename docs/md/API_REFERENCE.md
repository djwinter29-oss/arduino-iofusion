# API Reference (Arduino)

This document describes the current public API contract for IOFusion.

## Scope statement

- Target: Arduino ecosystem (Uno-class targets).
- Policy: keep API/implementation simple and stable for Arduino use cases.
- Non-goal for now: generic cross-platform HAL abstraction.

## Design contract

- **ISR-safe entry points**: `onTick()` style methods should stay minimal.
- **Loop-side processing**: heavier math/formatting runs in loop-side methods.
- **Non-blocking expectation**: call update methods frequently from `loop()`.

---

## `AnalogSampler`

Header: `lib/IOFusion/include/analog.h`

### Methods

- `bool begin(const uint8_t* channels, uint8_t count)`
  - Copies analog channel IDs (`0..5`) into internal storage.
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

## `DigiIn`

Header: `lib/IOFusion/include/digiin.h`

### Methods

- `bool begin(const uint8_t* pins, uint8_t count, uint16_t windowTicks=1000, float tickHz=1000.0f, bool usePullup=false)`
  - Initializes digital monitoring.
  - Returns `false` on invalid arguments or pin mapping failure.

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

Header: `lib/IOFusion/include/encoder.h`

### Methods

- `bool begin(uint8_t pinA, uint8_t pinB, uint8_t up, uint8_t down)`
  - Configures quadrature outputs (`pinA`, `pinB`) and direction inputs (`up`, `down`).

- `void onTick()`
  - ISR-side state advance based on direction input levels.

- `int32_t getPosition()`
- `bool getDirection()`
  - Direction semantics: `true` => UP, `false` => DOWN.

- `void reset()`
  - Resets position/state and drives outputs low.

---

## `Timer1PWM`

Header: `lib/IOFusion/include/pwm.h`

### Methods

- `bool begin(float freqHz)`
  - Configures Timer1 PWM frequency.

- `void setDuty(uint8_t channel, float percent)`
  - Channel `0`/`1`, duty in `0..100` (input is clamped).

- `void stop()`
  - Disables PWM outputs and clears setup state.

---

## `Timer2Driver`

Header: `lib/IOFusion/include/timer.h`

### Methods

- `uint16_t beginHz(float freqHz)`
  - Starts Timer2 near requested frequency.
  - Returns OCR value used (or `0` in test stubs).

- `void stop()`
- `void attachCallback(Timer2Callback cb)`
- `void detachCallback(Timer2Callback cb)`
- `static void handleInterrupt()`

---

## `CmdLine` command surface (firmware serial)

Source: `src/cmdline.cpp`

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
