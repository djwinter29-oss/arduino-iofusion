# IOFusion Examples

This file lists quick-start examples included in this repository.

## Available sketches

- `examples/basic_usage/basic_usage.ino`
  - Minimal initialization of AnalogSampler, DigiIn, EncoderGenerator, and Timer1PWM.

- `examples/frequency_monitor/frequency_monitor.ino`
  - Measures digital frequency/duty using `DigiIn` + `Timer2Driver` ISR tick.

- `examples/pwm_dual_channel/pwm_dual_channel.ino`
  - Drives Timer1 PWM on Uno pins 9/10 and sweeps dual-channel duty cycles.

- `examples/encoder_signal_generator/encoder_signal_generator.ino`
  - Generates quadrature A/B output and reports direction/position.

## Upload notes

Use your target environment in `platformio.ini` (default `uno`) and run:

```bash
pio run -e uno -t upload
pio device monitor -b 115200
```

## Wiring quick notes (Arduino Uno)

- `basic_usage`
  - Analog: A0, A1
  - Digital input monitor: D2, D3
  - Encoder output: A=D8, B=D11, direction controls: UP=D12, DOWN=D13
  - PWM output: CH0=D9, CH1=D10

- `frequency_monitor`
  - Measured inputs: D2, D3 (configured as `INPUT_PULLUP`)

- `pwm_dual_channel`
  - PWM outputs: CH0=D9 (OC1A), CH1=D10 (OC1B)

- `encoder_signal_generator`
  - Quadrature outputs: A=D8, B=D11
  - Direction controls (pull-up inputs): UP=D12, DOWN=D13

## Electrical notes

- `frequency_monitor` and `encoder_signal_generator` use input pins configured as `INPUT_PULLUP`.
- For deterministic results, avoid heavy blocking work in `loop()`.
