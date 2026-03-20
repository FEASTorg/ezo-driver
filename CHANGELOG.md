# Changelog

All notable tracked changes to this rewrite will be recorded here.

## [0.1.0] - current baseline

### Added

- C99 transport-agnostic core for EZO command formatting, response decoding, and numeric helpers
- shared `ezo.h` public surface for result types, timing hints, and numeric parsing
- UART C core with line-based response framing and response classification
- thin header-only C++11 wrapper over the I2C C core
- Arduino `TwoWire` transport adapter for I2C
- Linux file-descriptor transport adapter for I2C
- raw response API alongside the I2C text response path
- focused Arduino and Linux examples for the I2C path
- host-side C and C++ tests with fake transport coverage
- Linux adapter behavior tests
- GitHub Actions CI for host builds/tests
- PlatformIO Arduino compile CI for `uno`, `nanoatmega328`, and `esp32dev`

### Changed

- extracted shared formatting and parsing helpers into `ezo_common`
- simplified the repo so `src/` is the canonical library root for public headers and Arduino-safe implementation
- reduced tracked docs to a small handoff-oriented set

### Notes

- Arduino IDE validation is manual only
- `_reference/` remains legacy reference material and is not part of the product surface
- UART platform adapters are intentionally deferred past the current baseline
