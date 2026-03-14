# Coverage Limitations (Native Unit Tests)

This project uses a `native` test environment with Arduino mocks to maximize fast feedback and branch coverage.

## What is intentionally not fully covered

1. **AVR hardware register paths (`avr_timer1_pwm.cpp`, `avr_timer2_driver.cpp` non-`UNIT_TEST` sections)**
   - The production implementations write/read MCU registers (e.g., `TCCR1A`, `TCCR2B`, ISR vectors).
   - These paths require a real AVR target (or an accurate MCU simulator), not the host-native runtime.

2. **Real interrupt timing behavior**
   - Native tests call methods directly and cannot reproduce real interrupt latency/jitter behavior.
   - Timing-sensitive characteristics should be validated on hardware-in-the-loop tests.

3. **Physical IO electrical behavior**
   - Pull-up resistor behavior, input signal quality, and timer waveform quality depend on board/electrical setup.
   - Native mocks validate logic contracts, not electrical correctness.

## Why this is acceptable

- Native tests cover most loop-side business logic and error-handling branches, including:
   - `FirmwareCli` parser behavior and timeout-based framing,
   - `Timer2Driver` ownership / callback lifecycle behavior in `UNIT_TEST` mode,
   - `DigitalInputMonitor`, `AnalogSampler`, and `EncoderGenerator` logic contracts.
- Hardware-coupled code is only partially exercised through API-level contract tests in `UNIT_TEST` mode.
   - The native PWM path validates CLI/API flows, input validation, and retune command handling through stubs.
   - It does **not** validate AVR register-level Timer1 behavior, waveform quality, or retune edge cases on real hardware.
- CI remains fast and deterministic, while hardware verification should cover the remaining MCU-specific risk.

## What the native suite does not prove

- `FirmwareCli` is covered for parsing and framing, but not for real serial timing or host/device transport behavior.
- `Timer2Driver` lifecycle is covered in the unit-test stub and at the API contract level, but not for true AVR interrupt timing.
- `Timer1PWM` command paths are covered, but frequency retuning and saturation behavior still require AVR or hardware-in-the-loop validation because the native build uses stubs for the register path.

## Suggested future improvement

- Add optional hardware smoke tests on a real Uno runner (or lab rig), focusing on:
  - Timer2 tick frequency sanity
   - PWM frequency retuning, duty saturation, and output validation on D9/D10
   - End-to-end CLI behavior on a real serial device
