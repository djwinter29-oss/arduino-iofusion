# IOFusion Examples

This file lists quick-start examples included in this repository.

## Available sketches

### basic_usage/basic_usage.ino

Minimal initialization of AnalogSampler, DigitalInputMonitor, EncoderGenerator, and Timer1PWM.

Expected serial output:

```text
Periodic status or JSON-style output after setup.
```

### frequency_monitor/frequency_monitor.ino

Measures digital frequency and duty using DigitalInputMonitor with a Timer2Driver ISR tick.

Expected serial output:

```json
{"d2":{"freq":1000.0,"duty":50.0},"d3":{"freq":500.0,"duty":25.0}}
```

Measurement model:

- Sampled estimator.
- Best suited to signals well below the sampling Nyquist limit.

### pwm_dual_channel/pwm_dual_channel.ino

Drives Timer1 PWM on Uno pins D9 and D10 and sweeps dual-channel duty cycles.

Expected serial output:

```json
{"dutyA":10.0,"dutyB":90.0}
```

The sketch emits an update about every 500 ms.

### encoder_signal_generator/encoder_signal_generator.ino

Generates quadrature A/B output and reports direction and position.

Expected serial output:

```json
{"direction":"UP","position":123}
```

## Upload notes

Use your target environment in `platformio.ini` (default `uno`) and run:

```bash
pio run -e uno -t upload
pio device monitor -b 115200
```

## Wiring quick notes (Arduino Uno)

These examples assume pin and timer assignments are planned ahead of time. IOFusion keeps the runtime lightweight and does not try to detect all cross-module wiring overlaps for you.

### basic_usage

- Analog: A0, A1
- Digital input monitor: D2, D3
- Encoder output: A=D8, B=D11
- Direction controls: UP=D12, DOWN=D13
- PWM output: CH0=D9, CH1=D10

### frequency_monitor

- Measured inputs: D2, D3
- Inputs are configured as `INPUT_PULLUP`.
- Review `tickHz` and `windowTicks` before using it as a frequency reference; this is not a hardware capture example.

### pwm_dual_channel

- PWM outputs: CH0=D9 (OC1A), CH1=D10 (OC1B)

### encoder_signal_generator

- Quadrature outputs: A=D8, B=D11
- Direction controls (pull-up inputs): UP=D12, DOWN=D13

## Electrical notes

- `frequency_monitor` and `encoder_signal_generator` use input pins configured as `INPUT_PULLUP`.
- For deterministic results, avoid heavy blocking work in `loop()`.
- For high-frequency or narrow-pulse measurements, prefer hardware capture or dedicated edge interrupts over `DigitalInputMonitor`.

## Troubleshooting

### No serial output

- Verify monitor baud is `115200`.
- Press reset after opening the serial monitor.

### Unexpected pin behavior

- Re-check Uno pin mapping in "Wiring quick notes".
- Ensure no other shield or peripheral is using D9 or D10.

### Flat frequency or duty values

- Confirm input signal wiring and common ground.
- Confirm `INPUT_PULLUP` logic level expectations.
