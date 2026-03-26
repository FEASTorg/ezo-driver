from __future__ import annotations

import os
from pathlib import Path

from setuptools import Extension, setup

try:
    from Cython.Build import cythonize
except ImportError as exc:  # pragma: no cover
    raise RuntimeError("Cython is required to build ezo-driver-py") from exc

SETUP_DIR = Path(__file__).resolve().parent
ROOT = SETUP_DIR.parents[1]
SRC_DIR = ROOT / "src"
LINUX_DIR = ROOT / "platform" / "linux"


def relpath(path: Path) -> str:
    return os.path.relpath(path, SETUP_DIR).replace(os.sep, "/")

core_sources = sorted(relpath(path) for path in SRC_DIR.glob("*.c"))
linux_sources = [
    relpath(LINUX_DIR / "ezo_i2c_linux_i2c.c"),
    relpath(LINUX_DIR / "ezo_uart_posix_serial.c"),
]

ext_modules = [
    Extension(
        "ezo_driver_py._core",
        ["src/ezo_driver_py/_core.pyx", *core_sources, *linux_sources],
        include_dirs=[relpath(SRC_DIR)],
        define_macros=[("_GNU_SOURCE", "1")],
        language="c",
    )
]

setup(
    ext_modules=cythonize(ext_modules, compiler_directives={"language_level": "3"}),
)
