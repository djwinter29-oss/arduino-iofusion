# IOFusion Architecture

## 1) Goals

IOFusion is a small Arduino-focused component set for **deterministic timing**, with a clear split between:

- **Scope policy:** Arduino-only (Uno-class targets).
- **Non-goal:** multi-platform HAL support in the current roadmap.
- See also: [ARCHITECTURE_ARDUINO_SCOPE.md](ARCHITECTURE_ARDUINO_SCOPE.md) for scope and roadmap priorities.

- **ISR path**: minimal, fast operations only (flags/counters/state updates)
- **Main loop path**: heavier work (ADC reads, window math, serial command processing)

This design helps keep interrupt latency low and behavior predictable.

---

## 2) Module Overview

### Timer2Driver (`lib/IOFusion/include/avr_timer2_driver.h`, `lib/IOFusion/src/avr_timer2_driver.cpp`)

- Configures Timer2 at a requested tick rate.
- Dispatches registered callbacks from ISR context.
- Used as the heartbeat for periodic sampling / state stepping.

### AnalogSampler (`lib/IOFusion/include/analog_sampler.h`, `lib/IOFusion/src/analog_sampler.cpp`)

- Tracks configured analog channels.
- `onTick()` marks sampling due.
- `sampleIfDue()` performs ADC reads in main context.
- Converts raw ADC to voltage using configurable `Vref`.

### DigitalInputMonitor (`lib/IOFusion/include/digital_input_monitor.h`, `lib/IOFusion/src/digital_input_monitor.cpp`)

- Samples digital pin states each tick.
- Accumulates edge/high counts over a time window.
- `updateIfReady()` computes frequency and duty cycle in main context.
- This is a sampled estimator, not a hardware input-capture block.

### EncoderGenerator (`lib/IOFusion/include/encoder_generator.h`, `lib/IOFusion/src/encoder_generator.cpp`)

- **Generates** quadrature A/B output patterns.
- Uses `up/down` input pins as direction control.
- Maintains logical position and direction state.

### Timer1PWM (`lib/IOFusion/include/avr_timer1_pwm.h`, `lib/IOFusion/src/avr_timer1_pwm.cpp`)

- Configures Timer1 PWM output (Uno OC1A/OC1B, pins 9/10).
- Controls output frequency and per-channel duty cycle.

### FirmwareCli + app wiring (`src/firmware_cli.*`, `src/main.cpp`)

- Integrates all modules into a serial-controlled firmware.
- Provides text commands for query/control.

---

## 3) Runtime Data Flow

1. `Timer2Driver` ISR fires at fixed interval.
2. ISR callbacks run short handlers:
   - `AnalogSampler::onTick()`
  - `DigitalInputMonitor::onTick()`
   - `EncoderGenerator::onTick()`
3. `loop()` performs deferred work:
   - `AnalogSampler::sampleIfDue()`
  - `DigitalInputMonitor::updateIfReady()`
   - command parsing and responses

This flow preserves deterministic sampling while avoiding expensive ISR operations.

---

## 4) Timing Contract

- Keep ISR callbacks short and non-blocking.
- Keep `loop()` responsive; long blocking code will delay deferred calculations.
- Choose digital measurement windows (`windowTicks`) based on required response speed vs. stability.

### DigitalInputMonitor Envelope And Aliasing Limits

`DigitalInputMonitor` measures digital signals by sampling them once per timer tick. It does not timestamp edges in hardware. That means:

- transitions faster than $\frac{\text{tickHz}}{2}$ alias,
- narrow pulses shorter than one tick may be missed,
- frequency resolution is $\frac{\text{tickHz}}{\text{windowTicks}}$ Hz,
- duty resolution is approximately $\frac{100}{\text{windowTicks}}\%$.

For practical use, keep target input frequencies comfortably below Nyquist. A conservative guidance point is $f_{in} \le \frac{\text{tickHz}}{4}$ when you care about both frequency and duty-cycle fidelity.

The default firmware wiring in [src/main.cpp](src/main.cpp) uses `tickHz = 10000` and `windowTicks = 500`, so the measurement window is 50 ms, frequency resolution is about 20 Hz, and duty resolution is about 0.2%. Higher-frequency or narrow-pulse measurements should move to hardware capture or dedicated edge interrupts.

---

## 5) Repository Layout

- Core library code:
  - `lib/IOFusion/include`
  - `lib/IOFusion/src`
- Firmware app:
  - `src/`
- Tests:
  - `test/`
- CI/docs tooling:
  - `.github/workflows/`
  - `Doxyfile`, `tools/`

---

## 6) Library Packaging Notes

The project is configured for PlatformIO Library Registry via `library.json` at repo root. The library build section points to:

- `includeDir`: `lib/IOFusion/include`
- `srcDir`: `lib/IOFusion/src`

This allows publishing only library-relevant code while keeping firmware app/test/CI files in the same repository.
