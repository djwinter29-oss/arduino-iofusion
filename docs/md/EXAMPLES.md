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

## Electrical notes

- `frequency_monitor` and `encoder_signal_generator` use input pins configured as `INPUT_PULLUP`.
- For deterministic results, avoid heavy blocking work in `loop()`.
