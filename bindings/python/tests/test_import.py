import pytest

from ezo_driver_py import (
    COMMAND_CALIBRATION,
    COMMAND_GENERIC,
    COMMAND_READ,
    COMMAND_READ_WITH_TEMP_COMP,
    EzoArgumentError,
    EzoError,
    I2CDeviceLinux,
    UARTDeviceLinux,
)


def test_symbols_exported():
    assert EzoError is not None
    assert I2CDeviceLinux is not None
    assert UARTDeviceLinux is not None


def test_constants_exported():
    assert COMMAND_GENERIC == "generic"
    assert COMMAND_READ == "read"
    assert COMMAND_READ_WITH_TEMP_COMP == "read_with_temp_comp"
    assert COMMAND_CALIBRATION == "calibration"


def test_i2c_invalid_constructor_arguments_raise_value_error():
    with pytest.raises(ValueError):
        I2CDeviceLinux(bus=-1, address=0x63)

    with pytest.raises(ValueError):
        I2CDeviceLinux(bus=1, address=0x80)


def test_uart_invalid_arguments_raise_before_open():
    with pytest.raises(ValueError):
        UARTDeviceLinux(path="/dev/ttyS0", baud=12345, read_timeout_ms=100)

    with pytest.raises(ValueError):
        UARTDeviceLinux(path="/dev/ttyS0", baud=9600, read_timeout_ms=0)
