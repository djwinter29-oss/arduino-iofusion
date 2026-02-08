# PlatformIO Arduino Uno Project

Basic PlatformIO project targeting the Arduino Uno (ATmega328P).

## IOFusion design

IOFusion is a small set of hardware helpers focused on deterministic, timer-driven sampling and signal generation:

- `Timer2Driver` provides a periodic ISR tick for scheduling fast tasks.
- `AnalogSampler` defers ADC reads to `loop()` while the ISR only sets a flag.
- `DigiIn` samples digital inputs in the ISR and computes frequency/duty in `loop()`.
- `EncoderGenerator` produces a quadrature output and tracks position/direction.
- `Timer1PWM` configures Timer1 PWM on OC1A/OC1B (pins 9/10).

### Data flow


```mermaid
flowchart TD
    T2[Timer2Driver ISR] --> AS[AnalogSampler flag]
    T2 --> DI[DigiIn counters]
    T2 --> EN[EncoderGenerator state]

    LOOP[loop] --> AS
    LOOP --> DI
    LOOP --> CMD[Command parser]

    CMD --> PWM[Timer1 PWM]
    CMD --> RESP[Serial responses]

    AS --> RESP
    DI --> RESP
    EN --> RESP

```

### Timing model

- **ISR path:** minimal work only (flags + counters). No floating-point math.
- **Main loop:** performs ADC reads and computes frequency/duty, ensuring the ISR stays fast.

### Source layout

- Library headers: [lib/IOFusion/include](lib/IOFusion/include)
- Library sources: [lib/IOFusion/src](lib/IOFusion/src)
- Firmware entry: [src/main.cpp](src/main.cpp)
- Command line interface: [src/cmdline.h](src/cmdline.h) and [src/cmdline.cpp](src/cmdline.cpp)

## Command line interface

The firmware exposes a simple serial command line for querying sensors and controlling PWM. Commands are ASCII and return JSON-like responses.

Supported commands:

- `analog?` — returns analog voltages for configured channels.
- `digital?` — returns frequency and duty cycle for configured digital inputs.
- `encoder?` — returns encoder direction and position.
- `pwm-freq <hz>` — sets Timer1 PWM frequency.
- `pwm-duty <ch> <pct>` — sets PWM duty for channel 0 or 1.
- `help` — prints a short help string.

## Build and upload

Build with PlatformIO:

```bash
pio run
```

Upload to a connected Uno:

```bash
pio run --target upload
```
