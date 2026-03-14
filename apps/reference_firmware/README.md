# Reference Firmware

This folder contains the repository's reference firmware composition.

- `src/main.cpp` wires the IOFusion library modules together for an Arduino Uno-class target.
- `include/firmware_cli.h` and `src/firmware_cli.cpp` provide the serial command interface used by the reference firmware.

This code is intentionally separate from `lib/IOFusion/` so the library remains the primary integration surface for downstream users.