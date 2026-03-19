# EZO I2C Driver

Cross-platform EZO I2C driver rewrite with:

- a platform-agnostic C99 core
- a thin C++11 wrapper
- Arduino `TwoWire` integration
- Linux I2C adapter support
- host-side tests and Arduino compile CI

This repository is a rewrite informed by Atlas Scientific's original `Ezo_i2c_lib`, which remains preserved under [`_reference/`](/mnt/c/Users/Cameron/repos_feast/ezo-i2c-driver/_reference).

## Status

Current implementation includes:

- generic EZO command send/read flow
- text response decoding and numeric parsing
- Arduino and Linux transport adapters
- Arduino examples for both the C and C++ surfaces
- CMake host build/test flow
- PlatformIO Arduino compile validation in CI

## Repository Layout

- `include/`: canonical public headers
- `src/`: core implementation and Arduino-library-compatible forwarded headers
- `adapters/linux/`: Linux adapter implementation
- `examples/`: focused Arduino and Linux examples
- `tests/`: host-side tests and fakes
- `docs/`: tracked implementation-governing docs
- `_reference/`: canonical legacy reference material

## Build And Validation

Host validation uses CMake:

```sh
cmake -S . -B build -DEZO_BUILD_TESTS=ON -DEZO_BUILD_LINUX_ADAPTER=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

Arduino/PlatformIO validation is compile-only in CI.

Arduino IDE validation remains manual by design.

## Arduino And PlatformIO

Arduino-facing packaging metadata is provided via:

- [`library.properties`](/mnt/c/Users/Cameron/repos_feast/ezo-i2c-driver/library.properties)
- [`library.json`](/mnt/c/Users/Cameron/repos_feast/ezo-i2c-driver/library.json)

Examples:

- [`examples/arduino_smoke/arduino_smoke.ino`](/mnt/c/Users/Cameron/repos_feast/ezo-i2c-driver/examples/arduino_smoke/arduino_smoke.ino)
- [`examples/arduino_read/arduino_read.ino`](/mnt/c/Users/Cameron/repos_feast/ezo-i2c-driver/examples/arduino_read/arduino_read.ino)

## Contract Docs

Tracked implementation docs live in [`docs/`](/mnt/c/Users/Cameron/repos_feast/ezo-i2c-driver/docs):

- [`docs/decision-log.md`](/mnt/c/Users/Cameron/repos_feast/ezo-i2c-driver/docs/decision-log.md)
- [`docs/architecture.md`](/mnt/c/Users/Cameron/repos_feast/ezo-i2c-driver/docs/architecture.md)
- [`docs/api-contract.md`](/mnt/c/Users/Cameron/repos_feast/ezo-i2c-driver/docs/api-contract.md)
