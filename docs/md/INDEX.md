# Documentation Index

Start here for project documentation.

## Scope

IOFusion is maintained as an **Arduino-focused library**. Current development priority is reliability, API clarity, and release quality on Arduino/AVR targets rather than multi-platform abstraction.

## Core docs

- [ARCHITECTURE.md](ARCHITECTURE.md) — current module/runtime architecture.
- [ARCHITECTURE_ARDUINO_SCOPE.md](ARCHITECTURE_ARDUINO_SCOPE.md) — Arduino-only scope and near-term roadmap.
- [PLATFORMIO_LIBRARY.md](PLATFORMIO_LIBRARY.md) — install, usage, and publish guide.
- [EXAMPLES.md](EXAMPLES.md) — ready-to-run example sketches and pin notes.
- [API_REFERENCE.md](API_REFERENCE.md) — practical API contract and method behavior.
- [API_STABILITY.md](API_STABILITY.md) — compatibility guarantees, SemVer, and deprecation policy.
- [COVERAGE_LIMITATIONS.md](COVERAGE_LIMITATIONS.md) — what native tests can/cannot cover.
- [CONCURRENCY_MODEL.md](CONCURRENCY_MODEL.md) — ISR/loop ownership and synchronization rules.
- [RELEASE_AUTOMATION_PLAN.md](RELEASE_AUTOMATION_PLAN.md) — tag-driven GitHub Actions publish flow.

## Fast paths

- New user: read `PLATFORMIO_LIBRARY.md` then `EXAMPLES.md`.
- Contributor: read `ARCHITECTURE.md` then `COVERAGE_LIMITATIONS.md`.
- Maintainer (release): read `RELEASE_AUTOMATION_PLAN.md`.
