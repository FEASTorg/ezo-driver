# EZO I2C Driver

Small cross-platform EZO I2C driver rewrite with:

- a platform-agnostic C99 core
- a thin C++11 wrapper
- Arduino `TwoWire` integration
- Linux I2C adapter support
- host-side tests and Arduino compile CI

This repository is a rewrite informed by Atlas Scientific's original `Ezo_i2c_lib`. The legacy reference code remains under [`_reference/`](./_reference/).

## Status

Current implementation includes:

- generic EZO command send/read flow
- text and raw response decoding with numeric parsing helpers
- Arduino and Linux transport adapters
- Arduino examples for both the C and C++ surfaces
- CMake host build/test flow
- PlatformIO Arduino compile validation in CI

## Layout

- `src/`: canonical library source tree and public headers
- `examples/`: focused Arduino and Linux examples
- `tests/`: host-side tests and fakes
- `docs/`: tracked implementation docs
- `_reference/`: legacy reference material only

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

- [`library.properties`](./library.properties)
- [`library.json`](./library.json)

Examples:

- [`examples/arduino_smoke/arduino_smoke.ino`](./examples/arduino_smoke/arduino_smoke.ino)
- [`examples/arduino_read/arduino_read.ino`](./examples/arduino_read/arduino_read.ino)

## Docs

Tracked implementation docs:

- [`docs/architecture.md`](./docs/architecture.md)
- [`docs/api-contract.md`](./docs/api-contract.md)
