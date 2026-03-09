# Examples Overview

This folder contains runnable sketches for Arduino Uno-class targets.

## 1) `basic_usage/basic_usage.ino`

**When to use**
- Quick smoke test for all major modules together (`AnalogSampler`, `DigiIn`, `EncoderGenerator`, `Timer1PWM`).

**Wiring summary**
- Analog inputs: `A0`, `A1`
- Digital inputs: `D2`, `D3`
- Encoder outputs: `A=D8`, `B=D11`
- Direction controls: `UP=D12`, `DOWN=D13`
- PWM outputs (Timer1): `CH0=D9`, `CH1=D10`

**Expected behavior**
- Sketch initializes all modules in `setup()`.
- Internal periodic sampling/update is executed in `loop()`.
- PWM starts at 1 kHz with 25% (CH0) and 75% (CH1).

---

## 2) `frequency_monitor/frequency_monitor.ino`

**When to use**
- Measure frequency and duty cycle on digital inputs.

**Wiring summary**
- Inputs: `D2`, `D3`
- Timer source: Timer2 ISR at 2 kHz (internal)

**Expected serial output**
- Startup line: `frequency_monitor ready`
- Repeating JSON every ~250 ms, e.g.:
  - `{"d2":{"freq":1000.0,"duty":50.0},"d3":{"freq":500.0,"duty":25.0}}`

---

## 3) `encoder_signal_generator/encoder_signal_generator.ino`

**When to use**
- Generate quadrature encoder signals with direction control.

**Wiring summary**
- Encoder outputs: `A=D8`, `B=D11`
- Direction controls (INPUT_PULLUP): `UP=D12`, `DOWN=D13`
- Timer source: Timer2 ISR at 5 kHz (internal)

**Expected serial output**
- Startup line: `encoder_signal_generator ready`
- Repeating JSON every ~200 ms, e.g.:
  - `{"direction":"UP","position":1234}`

---

## 4) `pwm_dual_channel/pwm_dual_channel.ino`

**When to use**
- Drive two complementary PWM outputs using Timer1.

**Wiring summary**
- Channel 0: `D9` (OC1A)
- Channel 1: `D10` (OC1B)

**Expected serial output**
- Startup line: `pwm_dual_channel ready`
- Repeating JSON every ~500 ms, e.g.:
  - `{"dutyA":30.0,"dutyB":70.0}`

Duties sweep in opposite directions and stay complementary (`dutyA + dutyB = 100`).
