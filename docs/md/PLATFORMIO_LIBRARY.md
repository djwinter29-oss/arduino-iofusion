# IOFusion as a PlatformIO Library

This document follows PlatformIO library creation/publishing guidance and describes how to install, use, and release IOFusion.

## 1) Required Manifest

A PlatformIO library package must include a manifest at package root. IOFusion uses:

- `library.json` (recommended by PlatformIO)

Current manifest defines:

- name/version/description/keywords
- repository/homepage/license/authors
- supported frameworks/platforms
- build mapping to real source folders:
  - `lib/IOFusion/include`
  - `lib/IOFusion/src`

## 2) Scope and Compatibility

This library is intentionally maintained for Arduino-centric usage.

- Primary target: Arduino Uno-class boards.
- Cross-platform support is not currently promised.

## 3) Recommended Structure

PlatformIO recommends `include/` + `src/` in the library package. In this mono-repo, equivalent folders are mapped through `library.json` build settings:

- `build.includeDir = lib/IOFusion/include`
- `build.srcDir = lib/IOFusion/src`

Examples are provided under:

- `examples/basic_usage/basic_usage.ino`

## 4) Installation

### A. Install from Git URL (immediate use)

In consumer project `platformio.ini`:

```ini
lib_deps =
  https://github.com/djwinter29-oss/arduino-iofusion.git
```

### B. Install from PlatformIO Registry (after publish)

In consumer project `platformio.ini`:

```ini
lib_deps =
  djwinter29-oss/IOFusion
```

Or pin version:

```ini
lib_deps =
  djwinter29-oss/IOFusion@^0.1.0
```

## 5) Basic Usage

```cpp
#include <Arduino.h>
#include "analog_sampler.h"
#include "avr_timer1_pwm.h"
#include "digital_input_monitor.h"
#include "encoder_generator.h"

AnalogSampler analogSampler;
DigitalInputMonitor digitalInputMonitor;
EncoderGenerator encoder;
Timer1PWM pwm;

void setup() {
  const uint8_t analogPins[] = {0, 1};
  const uint8_t digitalPins[] = {2, 3};

  analogSampler.begin(analogPins, 2);
  analogSampler.setVref(5.0f);

  digitalInputMonitor.begin(digitalPins, 2, 500, 1000.0f, true);
  encoder.begin(8, 11, 12, 13, true, false);

  pwm.begin(1000.0f);
  pwm.setDuty(0, 25.0f);
  pwm.setDuty(1, 75.0f);
}

void loop() {
  analogSampler.onTick();
  digitalInputMonitor.onTick();
  encoder.onTick();

  analogSampler.sampleIfDue();
  digitalInputMonitor.updateIfReady();
}
```

See full example: `examples/basic_usage/basic_usage.ino`

## 6) Publish Process

1. Ensure `library.json` metadata is complete and correct.
2. Update `version` for every release.
3. Authenticate PlatformIO CLI if needed.
4. Publish:

```bash
pio pkg publish --type library
```

5. Verify page in PlatformIO Registry.

## 7) New Release Checklist

- [ ] Code/tests/docs updated
- [ ] CI green
- [ ] `library.json` version bumped
- [ ] Tag/release note prepared (optional but recommended)
- [ ] `pio pkg publish --type library` executed

## 8) Unpublish (if a version is bad)

```bash
pio pkg unpublish --type library --owner djwinter29-oss --name IOFusion --version <x.y.z>
```
