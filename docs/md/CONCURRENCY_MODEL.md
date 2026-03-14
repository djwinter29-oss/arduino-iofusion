# Concurrency Model (ISR vs loop)

This project uses a split execution model:

- **ISR context** (`onTick()`, timer interrupt callbacks): keep work minimal and deterministic.
- **loop context** (`sampleIfDue()`, `updateIfReady()`, command handling): do heavier math/formatting.

This document defines ownership and synchronization rules for shared state.

## Rules

1. ISR should only do quick sampling/counter updates and flag setting.
2. Shared multi-byte data accessed across contexts must be protected with `noInterrupts()/interrupts()`.
3. Loop-side code should copy ISR counters into local snapshots, then clear ISR counters inside one critical section.
4. Avoid dynamic allocation and long operations in ISR.

---

## Module map

### `AnalogSampler`

- **ISR-owned writes**: `_sampleRequested` via `onTick()`.
- **loop-owned writes**: `_lastValues[]` via `sampleIfDue()`.
- **shared fields**:
  - `_sampleRequested` (ISR sets, loop clears)
- **protection**:
  - `begin()` resets `_sampleRequested` in a critical section.
  - `sampleIfDue()` clears `_sampleRequested` in a critical section before ADC work.

### `DigitalInputMonitor`

- **ISR-owned writes**: `_samplesInWindow`, `_edgeCnt[]`, `_highCnt[]`, `_lastState[]`, `_windowReady`.
- **loop-owned writes**: `_freq[]`, `_duty[]` (after snapshot copy).
- **shared fields**:
  - ISR counters/flags listed above.
- **protection**:
  - `updateIfReady()` snapshots and clears ISR counters inside one critical section.
  - `getFrequency()/getDutyCycle()` read with interrupts disabled.

### `EncoderGenerator`

- **ISR-owned writes**: `_position`, `_directionUp`, `_state`, output pin states in `onTick()`.
- **loop reads**: `getPosition()`, `getDirection()`.
- **protection**:
  - getters wrap reads with `noInterrupts()/interrupts()`.
  - `reset()` updates shared state inside a critical section.

### `Timer1PWM`

- **loop-owned writes**: `_dutyPercent[]`, timer registers.
- **shared with ISR**: hardware timer registers.
- **protection**:
  - `begin()/setDuty()/stop()` update timer registers in critical sections.

### `Timer2Driver`

- **ISR-owned reads/calls**: active driver pointer and that instance's callback slots via `handleInterrupt()`.
- **loop-owned writes**: active driver pointer via `beginHz()/stop()`, callback slots via `attachCallback()/detachCallback()`.
- **protection**:
  - active-driver handoff and callback table updates are guarded by `noInterrupts()/interrupts()`.

### `main.cpp` runtime flags

- `analogOk`, `digiOk`, `encoderOk` are read by timer callback and written in setup.
- They are treated as shared ISR/loop state and declared `volatile`.

---

## Notes for contributors

- If you add new ISR/loop shared state, update this document and tests in the same PR.
- Prefer adding tiny helper functions for repeated read/parse logic to keep ISR paths clear.
