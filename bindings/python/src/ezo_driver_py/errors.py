class EzoError(Exception):
    """Base exception for ezo-driver Python bindings."""


class EzoArgumentError(EzoError):
    """Invalid argument passed by caller."""


class EzoTransportError(EzoError):
    """Transport-layer failure (I2C/UART IO problems)."""


class EzoProtocolError(EzoError):
    """Protocol error from the EZO device/core."""


class EzoParseError(EzoError):
    """Parse error when decoding device payloads."""
