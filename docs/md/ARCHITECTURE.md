# IOFusion Architecture

## Scope and Intent

IOFusion is an Arduino-focused library for deterministic sampling and signal generation on Uno-class targets.

- Primary goal: reliable Arduino behavior with explicit pin/timer configuration.
- Current non-goal: cross-platform HAL abstraction.
- Engineering priorities: API clarity, test quality, documentation quality, and reliable release automation.

The public product boundary is the library under `lib/IOFusion/`. The code under `apps/reference_firmware/` is a reference composition and test vehicle, not the primary downstream integration surface.

## Architectural Split

The runtime model is intentionally split between interrupt-side bookkeeping and loop-side work.

- ISR path: minimal, deterministic operations only.
- Loop path: ADC reads, window math, CLI parsing, and serial formatting.

This keeps interrupt latency low while still allowing useful higher-level behavior.

## Module Overview

### Timer2Driver

- Header: `lib/IOFusion/include/avr_timer2_driver.h`
- Source: `lib/IOFusion/src/avr_timer2_driver.cpp`
- Role: owns the periodic Timer2 tick and dispatches registered callbacks from ISR context.
- Contract: Timer2 frequency is chosen at startup; runtime retuning is intentionally disallowed until `stop()` releases the timer.

### AnalogSampler

- Header: `lib/IOFusion/include/analog_sampler.h`
- Source: `lib/IOFusion/src/analog_sampler.cpp`
- Role: marks analog sampling due in ISR context and performs ADC reads in `loop()`.

### DigitalInputMonitor

- Header: `lib/IOFusion/include/digital_input_monitor.h`
- Source: `lib/IOFusion/src/digital_input_monitor.cpp`
- Role: samples digital inputs once per tick, accumulates counts in ISR context, and computes frequency/duty in `loop()`.
- Constraint: sampled estimator only; it is not a hardware input-capture block.

### EncoderGenerator

- Header: `lib/IOFusion/include/encoder_generator.h`
- Source: `lib/IOFusion/src/encoder_generator.cpp`
- Role: generates quadrature A/B output steps from `up` and `down` level inputs.

### Timer1PWM

- Header: `lib/IOFusion/include/avr_timer1_pwm.h`
- Source: `lib/IOFusion/src/avr_timer1_pwm.cpp`
- Role: drives Uno Timer1 PWM outputs on D9/D10 with configurable frequency and duty.

### Reference Firmware

- Header: `apps/reference_firmware/include/firmware_cli.h`
- Sources: `apps/reference_firmware/src/main.cpp`, `apps/reference_firmware/src/firmware_cli.cpp`
- Role: composes the library into a serial-driven reference application.
- Intent: supports both occasional manual diagnostics over Serial and low-rate host polling, such as a Python app requesting fresh telemetry about once per second.

## Configuration Model

Each public component exposes a typed `Config` struct. That keeps wiring, timing, pull-up policy, and frequency choices explicit without hiding Arduino-specific details behind another abstraction layer.

Convenience overloads remain available, but config structs are the preferred setup surface.

Across the library, `begin()` is intended as a board-startup operation: configure the components in `setup()`, then let ISR and `loop()` code run against that fixed configuration during normal operation.

To stay small and predictable on Uno-class targets, the library assumes these assignments are decided up front by the application. IOFusion does not do exhaustive runtime reconciliation of overlapping pin maps or timer ownership across modules, and it does not use dynamic allocation or runtime resource negotiation to resolve such conflicts. Non-overlapping wiring and timer planning are therefore caller responsibilities.

## Runtime Data Flow

1. `Timer2Driver` fires at a fixed cadence.
2. ISR callbacks do only short state updates:
   - `AnalogSampler::onTick()`
   - `DigitalInputMonitor::onTick()`
   - `EncoderGenerator::onTick()`
3. `loop()` performs deferred work:
   - `AnalogSampler::sampleIfDue()`
   - `DigitalInputMonitor::updateIfReady()`
   - CLI processing and serial responses

The intended lifecycle is therefore: perform `begin()` calls during board startup, attach the Timer2 scheduler, and then treat the runtime as steady-state sampling/generation rather than a dynamic reconfiguration system.

## Concurrency Rules

This repository uses a strict ISR-versus-loop ownership model.

1. ISR code should only set flags, update counters, or step tiny state machines.
2. Shared multi-byte state must be protected with `noInterrupts()/interrupts()`.
3. Loop-side code should snapshot and clear ISR-owned counters inside one critical section.
4. Dynamic allocation and long-running operations do not belong in ISR code.

### Shared-State Ownership By Module

`AnalogSampler`

- ISR-owned writes: sample-request flag.
- Loop-owned writes: sampled values.
- Protection: request flag is set/cleared inside critical sections.
- Semantics: the request flag is single-depth. If loop-side ADC work is still pending, additional ISR ticks coalesce rather than queueing multiple analog sweeps.

`DigitalInputMonitor`

- ISR-owned writes: sample count, edge counters, high counters, last-state cache, window-ready flag.
- Loop-owned writes: computed frequency and duty arrays.
- Protection: `updateIfReady()` snapshots and clears ISR counters inside one critical section.

`EncoderGenerator`

- ISR-owned writes: position, direction, waveform state, output pins.
- Loop-side reads: `getPosition()`, `getDirection()`.
- Protection: getters and `reset()` use critical sections.
- Position contract: absolute count relative to startup or the most recent `reset()`, saturating at `int32_t` limits instead of wrapping.

`Timer1PWM`

- Loop-owned writes: duty cache and timer register programming.
- Protection: register changes are wrapped in critical sections.

`Timer2Driver`

- ISR-owned reads/calls: active-driver pointer and callback table.
- Loop-owned writes: driver activation and callback registration.
- Protection: ownership handoff and callback-table mutations are guarded.

## Timing Contract

- Keep ISR callbacks short and non-blocking.
- Keep `loop()` responsive; blocking code delays deferred work.
- Size `windowTicks` and `tickHz` around the signal envelope you actually care about.

### DigitalInputMonitor Limits

`DigitalInputMonitor` measures by sampling once per tick rather than timestamping physical edges. That means:

- transitions faster than $\frac{\text{tickHz}}{2}$ alias,
- pulses shorter than one tick can be missed,
- frequency resolution is $\frac{\text{tickHz}}{\text{windowTicks}}$ Hz,
- duty resolution is approximately $\frac{100}{\text{windowTicks}}\%$.

For robust duty and edge estimation, a practical target is $f_{in} \le \frac{\text{tickHz}}{4}$.

The default reference firmware wiring in [apps/reference_firmware/src/main.cpp](apps/reference_firmware/src/main.cpp) uses `tickHz = 10000` and `windowTicks = 500`, which yields a 50 ms window, about 20 Hz frequency resolution, and about 0.2% duty resolution.

In the reference firmware, that 10 kHz scheduler is intentionally not used to request an analog sweep on every tick. The analog path is treated as best-effort loop-side work and is decimated to a lower request rate so the six-channel ADC sweep remains physically achievable on an Uno.

## Repository Layout

- Core library: `lib/IOFusion/include`, `lib/IOFusion/src`
- Reference firmware: `apps/reference_firmware/include`, `apps/reference_firmware/src`
- Tests: `test/`
- Tooling and CI: `.github/workflows/`, `tools/`, `Doxyfile`

## Packaging Notes

PlatformIO packaging is driven from `library.json` at repo root.

- `build.includeDir = lib/IOFusion/include`
- `build.srcDir = lib/IOFusion/src`

That keeps the publishable package focused on the library while still allowing the repo to carry the reference firmware, tests, docs, and automation.
