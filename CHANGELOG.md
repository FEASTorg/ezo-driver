# Changelog

All notable tracked changes to this rewrite will be recorded here.

## [0.1.0] - current baseline

### Added

- C99 transport-agnostic core for EZO command formatting, response decoding, and numeric helpers
- thin header-only C++11 wrapper over the C core
- Arduino `TwoWire` transport adapter
- Linux file-descriptor transport adapter
- raw response API alongside the text response path
- focused Arduino and Linux examples
- host-side C and C++ tests with fake transport coverage
- Linux adapter behavior tests
- GitHub Actions CI for host builds/tests
- PlatformIO Arduino compile CI for `uno`, `nanoatmega328`, and `esp32dev`

### Changed

- simplified the repo so `src/` is the canonical library root for public headers and implementation
- reduced tracked docs to a small handoff-oriented set

### Notes

- Arduino IDE validation is manual only
- `_reference/` remains legacy reference material and is not part of the product surface
