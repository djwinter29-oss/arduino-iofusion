# IOFusion Architecture (AVR-First, Architecture-Ready)

This document defines a **low-risk roadmap**: keep AVR support stable now, prepare clean extension points for future boards.

## 1. Strategy

- **Current product promise**: AVR/Arduino Uno quality and stability.
- **Current roadmap decision**: stay Arduino-focused; do not pursue cross-platform HAL expansion now.
- **Near-term architecture goal**: improve API clarity, documentation, tests, and release quality.
- **Risk rule**: no behavior-breaking refactor until tests and interfaces are in place.

## 2. Why AVR-First

Hardware timers differ significantly across MCU families:

- Register layout and prescalers differ.
- ISR vector names and timer resources differ.
- PWM peripherals (Timer1 vs LEDC/TCC/etc.) differ.

Trying to unify everything in one pass increases failure risk. AVR-first keeps current value intact.

## 3. Target Layering

### 3.1 Module Layer (stable API)

- `AnalogSampler`
- `DigiIn`
- `EncoderGenerator`
- `Timer1PWM`
- `CmdLine`

These modules should expose user-facing behavior.

### 3.2 HAL Layer (per-architecture implementation)

Planned interfaces:

- `ITickTimer` (start/stop/attach callback)
- `IPwmDriver` (begin/setDuty/stop)
- Optional `IGpioFastIO` helpers

Planned backends:

- `hal/avr/*` (first)
- `hal/esp32/*` (future)
- `hal/samd/*` (future)

## 4. Incremental Migration Plan

1. **Document-only phase** (done): architecture and constraints.
2. **No-behavior-change extraction**:
   - move AVR register code behind HAL wrappers.
   - keep current public APIs unchanged.
3. **Compatibility phase**:
   - add compile-time backend selection by architecture macros.
4. **Single-board pilot**:
   - add one non-AVR board with smoke tests.
5. **Broaden support** only after pilot proves stable.

## 5. Timer Resource Policy

Per board family, maintain a resource map:

- which timers are used by IOFusion,
- known conflicts (Servo/tone/core libs),
- configurable alternatives where possible.

This avoids hidden runtime conflicts and improves portability.

## 6. Testing Policy

- Native tests: logic correctness and branch coverage.
- Hardware smoke tests: timer frequency, PWM output, and ISR behavior on real boards.

A platform is considered "supported" only after both pass.
