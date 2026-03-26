# Python Bindings Closeout Sweep (Post-Merge)

Status date: 2026-03-26

## Context

The initial Linux Python bindings are merged and CI-green for editable install and smoke tests.

Current shipped scope is intentionally low-level:

- Linux transport classes (`I2CDeviceLinux`, `UARTDeviceLinux`)
- raw command/send/read operations with timing hints
- typed Python exception mapping and constants
- Python CI install + smoke tests

## What is still missing before we can call the feature fully closed

## 1) Public API completeness gap (highest priority)

The Python layer does **not** yet expose parity wrappers for the full supported C surface:

- shared control module (`ezo_control.*`)
- calibration transfer module (`ezo_calibration_transfer.*`)
- typed product modules (`ezo_ph.*`, `ezo_orp.*`, `ezo_rtd.*`, `ezo_ec.*`, `ezo_do.*`, `ezo_hum.*`)

Closeout requirement:

- add Python modules that mirror these C APIs in transport-explicit shape
- include parse/output helpers where the C API already provides them
- keep no-hidden-retry/no-hidden-sleep behavior

## 2) Test depth gap

Current tests are smoke-level and constructor validation.

Closeout requirement:

- add unit tests per module for representative success/error paths
- add parser tests for typed reading outputs
- add parity tests against existing C fake-transport scenarios for key commands

## 3) Packaging hardening gap

Current build works with wrapper C files in `bindings/python/csrc`, but release-quality packaging still needs:

- sdist validation (`python -m build` then install from sdist)
- wheel validation (`python -m build -w`) on Linux
- explicit package data/include manifest checks

Closeout requirement:

- add a CI matrix for sdist + wheel build and install smoke from artifacts

## 4) Documentation closeout gap

Current README is intentionally short.

Closeout requirement:

- add full usage docs for I2C and UART flows
- add control/calibration/typed product examples when wrappers land
- add C-to-Python API mapping table for maintainability
- add Linux troubleshooting page (permissions, udev/groups, serial path tips)

## 5) Release policy gap

No formal release plan is documented for `ezo-driver-py` artifacts.

Closeout requirement:

- pick package publishing strategy (tag-only releases)
- define version mapping policy vs core repo version
- define support statement for architectures (x86_64, aarch64)

## Recommended closeout sequence (minimal risk)

## PR A — API parity foundation

- add Python modules for `control` + `calibration_transfer`
- add tests for common control workflows (info/status/led/protocol lock/response code)

## PR B — Typed product wrappers

- add scalar products first (`ph`, `orp`, `rtd`)
- then multi-output products (`ec`, `do`, `hum`)
- include parser and enabled-mask test cases

## PR C — Packaging + docs + release hardening

- artifact build matrix (sdist/wheel)
- install-from-artifact smoke checks
- user docs + API mapping table
- release checklist doc

## Exit criteria to mark the feature fully closed

1. Python API coverage includes control, transfer, and all six typed product modules.
2. Python tests cover representative success/error/parse paths for every exposed module.
3. CI builds and validates both sdist and wheel artifacts.
4. Public docs include transport quickstarts plus typed-module examples.
5. Release/version policy is documented and adopted.
