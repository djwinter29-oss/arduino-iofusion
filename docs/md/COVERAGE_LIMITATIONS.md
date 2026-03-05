# Coverage Limitations (Native Unit Tests)

This project uses a `native` test environment with Arduino mocks to maximize fast feedback and branch coverage.

## What is intentionally not fully covered

1. **AVR hardware register paths (`pwm.cpp`, `timer.cpp` non-`UNIT_TEST` sections)**
   - The production implementations write/read MCU registers (e.g., `TCCR1A`, `TCCR2B`, ISR vectors).
   - These paths require a real AVR target (or an accurate MCU simulator), not the host-native runtime.

2. **Real interrupt timing behavior**
   - Native tests call methods directly and cannot reproduce real interrupt latency/jitter behavior.
   - Timing-sensitive characteristics should be validated on hardware-in-the-loop tests.

3. **Physical IO electrical behavior**
   - Pull-up resistor behavior, input signal quality, and timer waveform quality depend on board/electrical setup.
   - Native mocks validate logic contracts, not electrical correctness.

## Why this is acceptable

- Native tests cover most business logic and error-handling branches.
- Hardware-coupled code is isolated and exercised through API-level contract tests in `UNIT_TEST` mode.
- CI remains fast and deterministic, while hardware verification can be added as a separate pipeline/job.

## Suggested future improvement

- Add optional hardware smoke tests on a real Uno runner (or lab rig), focusing on:
  - Timer2 tick frequency sanity
  - PWM frequency/duty output validation
  - End-to-end CLI behavior on serial device
