# ezo-driver-py

Optional Linux Python bindings for `ezo-driver`.

## Install (editable)

```bash
cd bindings/python
python -m pip install -e .
```

## Quick start

```python
from ezo_driver_py import I2CDeviceLinux

dev = I2CDeviceLinux(bus=1, address=0x63)
hint = dev.send_read()
print("wait(ms):", hint)
status, text = dev.read_response()
print(status, text)
dev.close()
```

## Scope in this initial implementation

- Linux I2C and POSIX UART raw transport access
- command send/read primitives with timing hints
- explicit transport separation and explicit lifecycle
- typed Python exception hierarchy for argument/transport/protocol/parse errors
- exported command/response/status constants for application-level logic

## Production contract for this binding layer

This package is intentionally transport-explicit and low-level:

- it mirrors the core command/read primitives and timing-hint behavior
- it does **not** add implicit delays, retries, or reconnect workflows
- it keeps I2C and UART as separate device classes
