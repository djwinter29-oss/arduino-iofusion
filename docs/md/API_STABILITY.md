# API Stability Policy

## Scope

This policy applies to IOFusion's Arduino-focused public API.

## Stability Levels

### Stable API

Public headers under `lib/IOFusion/include/` are considered stable unless explicitly documented otherwise.

- Backward compatibility is expected within minor/patch releases.
- Breaking changes should only happen in a major version bump.

### Internal API

Implementation details under `lib/IOFusion/src/` and reference firmware wiring under `apps/reference_firmware/` are internal.

- May change without notice.
- Not intended as integration surface for downstream users.

### Experimental API

Any new API marked as experimental in docs/changelog may change between minor releases.

## Versioning Rules (SemVer)

- **PATCH** (`x.y.Z`): bug fixes, docs, tests, no intended public API breaks.
- **MINOR** (`x.Y.z`): backward-compatible API additions.
- **MAJOR** (`X.y.z`): breaking API changes.

## Deprecation Policy

- Deprecations are announced in release notes/docs before removal.
- Prefer at least one minor release deprecation window before removal.

## Compatibility Promise

Current project direction is Arduino-only (Uno-class targets). Compatibility commitments in this policy are made in that scope.
