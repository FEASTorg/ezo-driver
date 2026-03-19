# Src

Core C99 implementation files belong here.

The implementation in this directory must remain platform-agnostic and must not depend on Arduino or Linux-specific headers.

Exceptions:

- `src/arduino/` contains the Arduino adapter implementation so Arduino library tooling and PlatformIO can compile it as part of the library source set.
- forwarded public headers may be mirrored under `src/` when required by Arduino library layout rules. The canonical public headers remain under `include/`.
