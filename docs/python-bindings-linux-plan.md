# Python Bindings on Linux: Full Implementation Plan

Status: proposed  
Audience: maintainers and contributors  
Last updated: 2026-03-26

## 1) Objective

Ship **optional** Python bindings that provide full practical use of `ezo-driver` on Linux while preserving the current architecture:

- C99 core remains the source of truth
- explicit I2C vs UART transport split remains intact
- no hidden retries/sleeps/reconnect behavior is introduced
- no Python requirement is introduced for existing C/C++/Arduino users

## 2) Architectural constraints from this repo

This plan is intentionally shaped by the repo's existing rules and surface:

- C core + explicit transport families are canonical.
- Linux I2C and POSIX UART adapters already exist and are tested.
- the public API has broad typed product coverage (pH, ORP, RTD, EC, DO, HUM) and shared control/calibration modules.

Therefore, Python bindings should be a thin adapter layer over the existing C API, not a re-architecture.

## 3) Research-backed decisions

## 3.1 Build and packaging standards

- Use `pyproject.toml` + PEP 517 build backend model (modern, standard build workflow).
- Use PEP 621 metadata in `pyproject.toml`.
- Build Linux wheels with `cibuildwheel` and manylinux policy (PEP 600).

References:

- PEP 517: <https://peps.python.org/pep-0517/>
- PEP 621: <https://peps.python.org/pep-0621/>
- PEP 600: <https://peps.python.org/pep-0600/>
- PyPA package formats discussion: <https://packaging.python.org/en/latest/discussions/package-formats/>
- cibuildwheel platforms docs: <https://cibuildwheel.pypa.io/en/stable/platforms/>

## 3.2 Wrapper technology choice

**Recommendation: Cython-first implementation.**

Reasoning:

- This repo exposes a substantial C API with structs, enums, callbacks, and out-params.
- Cython's `cdef extern from` model maps this style directly and efficiently.
- Cython permits a thin layer without requiring a new C++ facade.

References:

- Cython external C code guide: <https://cython.readthedocs.io/en/stable/src/userguide/external_C_code.html>
- Cython C/C++ wrapping guides: <https://cython.readthedocs.io/en/stable/src/userguide/wrapping_CPlusPlus.html>

## 3.3 Why not cffi-first or pybind11-first

- `cffi` is viable, but its own docs note ABI mode fragility/perf tradeoffs; API mode is better but still less direct for this repo's desired thin, typed, API-wide mapping.
- `pybind11` is excellent for C++ APIs, but this codebase is C-first and intentionally thin in C++ convenience layers; a pybind11-first path would likely push an extra C++ abstraction layer.

References:

- CFFI overview (ABI vs API discussion): <https://cffi.readthedocs.io/en/stable/overview.html>
- pybind11 basics/install docs: <https://pybind11.readthedocs.io/en/stable/basics.html>, <https://pybind11.readthedocs.io/en/latest/installing.html>

## 4) Deliverable definition: what “full Linux use” means

The Python package must support all of the following on Linux:

1. Raw transport operations for I2C and UART.
2. Shared control/admin surface (info, name, LED, status, protocol lock, UART response-code mode, etc.).
3. Calibration transfer helpers.
4. Typed product helpers for all currently supported families:
   - `ph`, `orp`, `rtd`, `ec`, `do`, `hum`
5. Linux transport-backed construction paths:
   - Linux I2C bus/device access
   - POSIX serial UART access
6. Timing hints returned to caller (never auto-sleep in wrapper).

## 5) Proposed repository layout

```text
bindings/
  python/
    pyproject.toml
    README.md
    CMakeLists.txt
    src/
      ezo_driver_py/
        __init__.py
        _core.pyx
        _core.pxd
        errors.py
        types.py
        i2c.py
        uart.py
        control.py
        calibration_transfer.py
        ph.py
        orp.py
        rtd.py
        ec.py
        do.py
        hum.py
    tests/
      test_raw_i2c.py
      test_raw_uart.py
      test_control.py
      test_calibration_transfer.py
      test_products_scalar.py
      test_products_multi.py
      test_linux_adapters.py
```

Notes:

- Keep this subtree optional and self-contained.
- Root build behavior remains unchanged unless explicitly opting into Python build/test jobs.

## 6) Public Python API design

Keep transport-explicit classes (no fake unified device):

- `I2CDeviceLinux(bus: int, address: int)`
- `UARTDeviceLinux(port: str, baud: int)`

Layered modules mirroring C API groups:

- `raw_i2c`, `raw_uart`
- `control`
- `calibration_transfer`
- typed product modules: `ph`, `orp`, `rtd`, `ec`, `do`, `hum`

Style rules:

- method names closely track C entry points for auditability
- structured Python return objects for parsed outputs (dataclasses)
- typed exceptions mapping from `ezo_result_t`
- no hidden retries/sleeps, no background state machine

## 7) Error model

Map `ezo_result_t` to Python exceptions:

- `EZO_OK` -> no exception
- argument/usage errors -> `ValueError` or `EzoUsageError`
- I/O/transport issues -> `EzoTransportError`
- parse/data-shape failures -> `EzoParseError`

Return device/status info explicitly when present (rather than silently normalizing to booleans).

## 8) Timing model

Each send operation that exposes timing hints in C should expose equivalent timing in Python (`timing_hint_ms` or equivalent enum/value object).

Policy:

- wrapper never sleeps automatically
- callers can opt into helper utilities (outside core API) if they want sleep behavior

## 9) Build system and wheel plan

## 9.1 Backend

Use `scikit-build-core` as backend for CMake + Python packaging integration.

Why:

- modern `pyproject.toml` backend
- strong CMake integration
- straightforward interop with wheel builds

Reference:

- scikit-build-core getting started: <https://scikit-build-core.readthedocs.io/en/stable/guide/getting_started.html>

## 9.2 Wheel strategy

Initial target matrix:

- Linux x86_64 and aarch64 wheels (manylinux policy)
- sdist for source builds on additional Linux environments

Tooling:

- `cibuildwheel` in CI for binary wheels

Reference:

- cibuildwheel platforms docs: <https://cibuildwheel.pypa.io/en/stable/platforms/>

## 9.3 Artifact boundaries

- Python package artifacts are produced only from `bindings/python/` flow.
- Existing CMake install/library flows remain unaffected.

## 10) Test strategy

## 10.1 Python-level unit tests

- API parity tests for representative calls in each module
- exception mapping tests
- typed parse/output tests for each product family

## 10.2 Integration tests (Linux)

- Linux adapter smoke tests for I2C and UART object construction paths
- mock/fake transport tests where hardware is unavailable

## 10.3 Cross-language parity checks

For selected scenarios, compare Python results against existing C test vectors to reduce drift.

## 10.4 CI plan

Add an **optional** Python CI job:

1. build extension
2. run Python tests
3. (release pipeline only) build wheels via cibuildwheel

Do not gate core C/C++ CI on Python packaging availability.

## 11) Documentation plan

Add docs in this order:

1. `bindings/python/README.md` quickstart
2. transport-explicit examples (I2C and UART)
3. typed product examples (all six families)
4. migration guidance: C API name to Python API name mapping table
5. troubleshooting (permissions, serial access, i2c-dev setup)

## 12) Implementation phases, acceptance criteria, and risks

## Phase 0 — Scaffolding

Tasks:

- create `bindings/python/` layout
- add `pyproject.toml`, CMake integration for extension build
- add minimal `import ezo_driver_py` smoke

Acceptance criteria:

- `pip install -e .` works in `bindings/python/`
- basic module import works on Linux CI

Risks:

- packaging complexity early on

Mitigation:

- keep initial extension surface minimal; add modules incrementally

## Phase 1 — Raw transport + Linux constructors

Tasks:

- expose I2C and UART raw send/read wrappers
- expose Linux-backed constructors
- establish error/timing mapping primitives

Acceptance criteria:

- raw command roundtrip tests pass
- timing hint API stable and documented

Risks:

- fd lifecycle bugs

Mitigation:

- explicit close semantics + context manager support

## Phase 2 — Control + calibration transfer

Tasks:

- bind shared control module
- bind calibration transfer module

Acceptance criteria:

- representative control and transfer workflows tested in Python
- no hidden behavioral changes from C contract

## Phase 3 — Typed product modules

Tasks:

- expose `ph`, `orp`, `rtd`, `ec`, `do`, `hum`
- add typed return objects and mask/config helpers where required

Acceptance criteria:

- product module coverage complete for existing FULL support matrix
- parser parity tests pass across all six families

## Phase 4 — Packaging hardening + release readiness

Tasks:

- wheel matrix (x86_64, aarch64)
- sdist validation
- user docs and examples finalized

Acceptance criteria:

- reproducible CI wheel builds
- install and smoke test from built wheel artifacts

## 13) Security and operational notes

- Document Linux permissions expectations (`/dev/i2c-*`, serial group membership).
- Ensure file descriptor ownership and cleanup are deterministic.
- Avoid implicit shelling-out from the package runtime.

## 14) Backward compatibility policy

- This addition is strictly additive and optional.
- No C API, ABI, or existing packaging behavior should be changed by default.

## 15) Open questions to resolve before implementation start

1. Package name on PyPI (`ezo-driver` vs `ezo-driver-py`)?
2. Minimum Python version (recommend 3.9+ unless a tighter constraint emerges)?
3. Whether first release should include both I2C and UART typed modules simultaneously, or staged.
4. Wheel publication policy (tag-only or every release).

## 16) Recommended next action

Begin with **Phase 0 + Phase 1 in one PR** to validate build/tooling and transport contracts end-to-end before binding all typed modules.

## 17) Production-ready definition of done

The Python bindings layer can be called production-ready when all are true:

1. Linux I2C and UART transport classes are stable and tested.
2. Packaging/install path is reproducible (`pyproject.toml`, CI install, import tests).
3. Public constants and exception semantics are documented and stable.
4. CI runs Python binding build + tests on every PR.
5. No behavior violates core constraints (no hidden retries/sleeps; explicit transport split).

---

## Appendix A — Reference links used for this plan

- PEP 517: <https://peps.python.org/pep-0517/>
- PEP 621: <https://peps.python.org/pep-0621/>
- PEP 600: <https://peps.python.org/pep-0600/>
- PyPA package formats discussion: <https://packaging.python.org/en/latest/discussions/package-formats/>
- Cython external C code docs: <https://cython.readthedocs.io/en/stable/src/userguide/external_C_code.html>
- Cython wrapping docs: <https://cython.readthedocs.io/en/stable/src/userguide/wrapping_CPlusPlus.html>
- CFFI overview: <https://cffi.readthedocs.io/en/stable/overview.html>
- pybind11 basics: <https://pybind11.readthedocs.io/en/stable/basics.html>
- pybind11 installing/build system notes: <https://pybind11.readthedocs.io/en/latest/installing.html>
- scikit-build-core getting started: <https://scikit-build-core.readthedocs.io/en/stable/guide/getting_started.html>
- cibuildwheel platform docs: <https://cibuildwheel.pypa.io/en/stable/platforms/>
