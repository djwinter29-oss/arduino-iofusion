# IOFusion Examples

This file lists quick-start examples included in this repository.

## Available sketches

- `examples/basic_usage/basic_usage.ino`
  - Minimal initialization of AnalogSampler, DigitalInputMonitor, EncoderGenerator, and Timer1PWM.
  - Expected serial: periodic status/JSON-style output after setup.

- `examples/frequency_monitor/frequency_monitor.ino`
  - Measures digital frequency/duty using `DigitalInputMonitor` + `Timer2Driver` ISR tick.
  - Expected serial: JSON lines like `{\"d2\":{\"freq\":... ,\"duty\":...},\"d3\":...}`.
  - Measurement model: sampled estimator, best suited to signals well below the sampling Nyquist limit.

- `examples/pwm_dual_channel/pwm_dual_channel.ino`
  - Drives Timer1 PWM on Uno pins 9/10 and sweeps dual-channel duty cycles.
  - Expected serial: JSON lines such as `{\"dutyA\":10.0,\"dutyB\":90.0}` every 500 ms.

- `examples/encoder_signal_generator/encoder_signal_generator.ino`
  - Generates quadrature A/B output and reports direction/position.
  - Expected serial: JSON lines like `{\"direction\":\"UP\",\"position\":123}`.

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
  - Review the configured `tickHz` and `windowTicks` before using it as a frequency reference; it is not a hardware capture example.

- `pwm_dual_channel`
  - PWM outputs: CH0=D9 (OC1A), CH1=D10 (OC1B)

- `encoder_signal_generator`
  - Quadrature outputs: A=D8, B=D11
  - Direction controls (pull-up inputs): UP=D12, DOWN=D13

## Electrical notes

- `frequency_monitor` and `encoder_signal_generator` use input pins configured as `INPUT_PULLUP`.
- For deterministic results, avoid heavy blocking work in `loop()`.
- For high-frequency or narrow-pulse measurements, prefer hardware capture or dedicated edge interrupts over `DigitalInputMonitor`.

## Troubleshooting (quick)

- No serial output:
  - Verify monitor baud is `115200`.
  - Press reset after opening serial monitor.
- Unexpected pin behavior:
  - Re-check Uno pin mapping in "Wiring quick notes".
  - Ensure no other shield/peripheral is using D9/D10 (Timer1 PWM pins).
- Flat frequency/duty values:
  - Confirm input signal wiring and common ground.
  - Confirm `INPUT_PULLUP` logic level expectations.
