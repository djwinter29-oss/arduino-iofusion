# IOFusion Arduino Scope and Roadmap

## Scope decision

IOFusion is intentionally maintained as an **Arduino-focused** library (Uno-class targets).

- Keep implementation and API simple/stable for Arduino usage.
- Do **not** pursue cross-platform HAL expansion in the current roadmap.

## Near-term priorities

1. API clarity and consistency
2. Test quality and coverage stability
3. Documentation quality and onboarding
4. Reliable release automation and versioning

## Non-goals (current phase)

- Generic multi-platform HAL abstraction
- Board-family expansion outside Arduino-focused scope

## Engineering guardrails

- Avoid behavior-breaking refactors without strong test coverage.
- Keep ISR-path logic minimal and deterministic.
- Prefer maintainability over abstraction complexity.
