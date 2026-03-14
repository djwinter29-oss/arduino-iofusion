# Documentation Index

Start here for project documentation.

## Recommended Reading Paths

- New library user: [PLATFORMIO_LIBRARY.md](PLATFORMIO_LIBRARY.md) then [EXAMPLES.md](EXAMPLES.md)
- Contributor: [ARCHITECTURE.md](ARCHITECTURE.md) then [API_REFERENCE.md](API_REFERENCE.md)
- Maintainer: [RELEASES.md](RELEASES.md) then [COVERAGE_LIMITATIONS.md](COVERAGE_LIMITATIONS.md)

## Core Documents

- [ARCHITECTURE.md](ARCHITECTURE.md) — scope, module boundaries, concurrency model, timing model, and repository layout
- [API_REFERENCE.md](API_REFERENCE.md) — public API surface, response contracts, and stability/deprecation policy
- [PLATFORMIO_LIBRARY.md](PLATFORMIO_LIBRARY.md) — install, packaging, publish, and registry usage notes
- [EXAMPLES.md](EXAMPLES.md) — runnable sketches, wiring summaries, and upload notes
- [RELEASES.md](RELEASES.md) — automated release flow, manual fallback, and verification steps
- [COVERAGE_LIMITATIONS.md](COVERAGE_LIMITATIONS.md) — what native unit tests prove and what still requires hardware validation

## Documentation Shape

The docs folder is intentionally small:

- architecture and concurrency live together because they describe the same runtime model,
- API behavior and stability policy live together because both define the public contract,
- release automation and manual fallback live together because they are part of one release process.
