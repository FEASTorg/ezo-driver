# Python Binding Cutover Plan

## Purpose

This document turns the reset architecture in `docs/python-binding-reset-architecture.md` into an implementation plan.

The plan assumes breaking cleanup is allowed.

## Cutover Principle

Do not incrementally refine the current large handwritten Python binding.

Keep the useful repo cleanup, then replace the implementation model.

## What To Keep

Keep these decisions unless a later implementation constraint forces a change:

- package name `ezo-driver`
- import package `ezo_driver`
- editable install entry point at `bindings/python/`
- Linux-only support statement for the Python package
- removal of the copied `bindings/python/csrc/` shim tree
- removal of the old `ezo_driver_py` package
- one canonical Python README at `bindings/python/README.md`

## What To Discard

Discard the current large handwritten binding implementation.

That means the replacement should not preserve the current internal design of:

- `bindings/python/src/ezo_driver/_binding.pyx`
- `bindings/python/src/ezo_driver/_converters.py`
- the current per-module wrapper bodies if they continue the same manual forwarding model
- the current Python tests if they are tied to that implementation shape instead of the reset shape

The target is a new implementation, not a cleanup pass on the current one.

## Deliverables

The cutover is complete when all of the following exist:

1. a small Linux host C facade for binding lifecycle
2. a `cffi` API-mode Python build
3. thin Python public modules with near-direct C parity
4. parity tests across the supported C headers
5. one canonical Python README that matches the shipped package

## Phase 0: Reset The Python Tree

### Goal

Freeze the current Python work and clear the way for the replacement.

### Tasks

- keep packaging identity changes already made
- keep docs cleanup that removed roadmap-style Python docs
- remove the current handwritten binding internals
- remove Cython as the primary implementation strategy for the Python package
- remove any stale tests that validate the wrong public shape

### Exit Criteria

- no large handwritten Cython bridge remains
- no copied C source tree remains under `bindings/python/`
- the repo still has one Python package entry point under `bindings/python/`

## Phase 1: Add The C Binding Seam

### Goal

Make the canonical C library easy to bind without rewriting its logic.

### Tasks

- add `src/ezo_linux_device.h`
- add the Linux host facade implementation under `platform/linux/`
- expose caller-owned `ezo_linux_i2c_device_t` and `ezo_linux_uart_device_t`
- add open and close helpers for both devices
- add core accessors returning `ezo_i2c_device_t *` and `ezo_uart_device_t *`
- add any missing Linux I2C open-by-bus or open-by-path helper needed for the Python device classes

### Required C Cleanup In This Phase

- fix the I2C text-response capacity contract
- fix UART oversized-line recovery behavior
- reject numeric overflow in `ezo_parse_double`
- add or clarify any helper that Python needs for explicit response or status handling

### Tests

- add host-facade unit tests beside the current Linux adapter tests
- expand existing Linux adapter tests if helper behavior moves

### Exit Criteria

- a host program can open, use, and close Linux I2C and UART devices through the new facade
- the existing core API remains intact
- the binding seam is free of Python-specific code

## Phase 2: Replace The Build With `cffi`

### Goal

Switch the Python package from a handwritten wrapper implementation to a declaration-driven build.

### Tasks

- add `bindings/python/build_ffi.py`
- define the canonical header list compiled into the extension
- add curated `cdef` files grouped by header family
- remove `Cython` build requirements from `bindings/python/pyproject.toml`
- update `bindings/python/setup.py` or equivalent package build entry to invoke `cffi`
- ensure editable install builds against the repo `src/` and `platform/linux/` sources directly

### Rules

- `cdef` declarations stay close to the C headers
- no vendored copy of the C library under `bindings/python/`
- no generated C wrapper sources checked into git

### Tests

- editable install from a clean checkout
- import of the compiled module on Linux host CI

### Exit Criteria

- the package builds with `python -m pip install -e bindings/python`
- the compiled module exposes `ffi` and `lib`
- the build no longer depends on the current large Cython wrapper

## Phase 3: Rebuild The Thin Python Runtime

### Goal

Reintroduce only the Python code that is actually justified.

### Tasks

- implement `errors.py` for `ezo_result_t` mapping
- implement `enums.py` for public enums and flags
- implement `types.py` for the small public POD result set
- implement `_ffi.py` as the runtime loader
- implement `_call.py` for the generic call shapes used across the library
- implement `i2c.py` and `uart.py` with `LinuxI2CDevice` and `LinuxUARTDevice`
- keep device classes responsible only for lifecycle and context manager support

### Rules

- no product logic in the device classes
- no dict-converter layer
- no second internal API shape
- no dynamic behavior that hides the underlying C contract

### Exit Criteria

- the package has the minimum runtime scaffolding needed to bind the C API cleanly
- the Python runtime layer is small and auditable

## Phase 4: Restore Shared Module Parity

### Goal

Restore parity for the non-product shared C headers first.

### Scope

- `base`
- `product`
- `parse`
- `schema`
- `control`
- `calibration_transfer`

### Tasks

- map functions directly from the corresponding C headers
- preserve `_i2c` and `_uart` suffixes in Python names
- return integer milliseconds for timing hints
- return public dataclasses for public POD structs
- return `str` or `bytes` instead of manual output buffers

### Tests

- one parity test file per shared header family
- representative success and failure cases for parse, build, query, and read helpers

### Exit Criteria

- every shared public C helper relevant to Linux use has a Python entry point
- the module layout matches the documented Python public surface

## Phase 5: Restore Product Module Parity

### Goal

Restore the full typed product surface over both transports.

### Order

1. `ph`
2. `orp`
3. `rtd`
4. `ec`
5. `do`
6. `hum`

### Tasks

- map every supported send, query, set, and read helper
- keep transport-explicit function names
- return typed public result objects where the C layer returns POD structs
- keep raw transport status or response-kind values explicit

### Tests

- parity tests by product family
- both I2C and UART paths where the C library supports both
- config and calibration flows, not only simple measurement reads

### Exit Criteria

- every supported public product header relevant to Linux use has Python parity
- the Python package does not omit command families that already exist in C

## Phase 6: Rebuild The Python Test Strategy

### Goal

Make the Python tests prove parity with the C library instead of proving wrapper behavior only.

### Tasks

- add a tiny Python test-only bridge over `tests/fakes/`
- reuse fake transports from the C test suite instead of inventing new Python fakes
- add tests for raw transport lifecycle and explicit status handling
- add tests for shared helper modules
- add tests for all six product modules
- add regression coverage for known C contract edge cases

### Minimum Regression Set

- I2C text/raw response capacity behavior
- UART oversized-line recovery
- decimal formatting validation
- parse overflow rejection
- explicit I2C status and UART response-kind propagation

### Exit Criteria

- the Python suite mirrors the supported C surface closely enough to catch parity drift
- CI green means more than import smoke

## Phase 7: Finalize Docs And Packaging

### Goal

Make the package publishable in shape, even if PyPI publishing is still deferred.

### Tasks

- rewrite `bindings/python/README.md` to match the reset architecture exactly
- keep the root README link short and canonical
- make sure no plan or closeout docs are linked as user-facing Python docs
- align Python package versioning with the repo version
- keep PyPI release automation out of scope until parity and tests are complete

### Exit Criteria

- repo docs describe the actual shipped Python package
- the package is structurally ready for future PyPI work

## File Plan

### New C Files

- `src/ezo_linux_device.h`
- `platform/linux/ezo_linux_device.c`
- tests for the new host facade

### New Python Build Files

- `bindings/python/build_ffi.py`
- `bindings/python/cdef/*.h`

### New Python Runtime Files

- `bindings/python/src/ezo_driver/_ffi.py`
- `bindings/python/src/ezo_driver/_call.py`
- `bindings/python/src/ezo_driver/errors.py`
- `bindings/python/src/ezo_driver/enums.py`
- `bindings/python/src/ezo_driver/types.py`
- `bindings/python/src/ezo_driver/i2c.py`
- `bindings/python/src/ezo_driver/uart.py`
- `bindings/python/src/ezo_driver/base.py`
- `bindings/python/src/ezo_driver/product.py`
- `bindings/python/src/ezo_driver/parse.py`
- `bindings/python/src/ezo_driver/schema.py`
- `bindings/python/src/ezo_driver/control.py`
- `bindings/python/src/ezo_driver/calibration_transfer.py`
- `bindings/python/src/ezo_driver/ph.py`
- `bindings/python/src/ezo_driver/orp.py`
- `bindings/python/src/ezo_driver/rtd.py`
- `bindings/python/src/ezo_driver/ec.py`
- `bindings/python/src/ezo_driver/do.py`
- `bindings/python/src/ezo_driver/hum.py`

### Files To Remove During Cutover

- `bindings/python/src/ezo_driver/_binding.pyx`
- `bindings/python/src/ezo_driver/_converters.py`
- any remaining Python implementation file whose only job is to compensate for the large handwritten wrapper model

## Acceptance Criteria

The cutover is done when all of the following are true:

- the Python package builds from editable install on Linux
- the Python package binds to canonical repo C sources directly
- the Python runtime layer is obviously thin on review
- the package exposes full parity for the supported C surface relevant to Linux use
- the tests cover shared helpers and all six product modules
- the README describes exactly the shipped Python surface

## Deferred Work

The following stays out of scope until the cutover is complete and stable:

- PyPI publishing workflow
- wheels for non-Linux platforms
- generator infrastructure beyond what is needed for the thin `cffi` binding
- higher-level workflow abstractions above the canonical C surface
