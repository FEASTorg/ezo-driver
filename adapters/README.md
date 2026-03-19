# Adapters

Platform integrations live here.

Each adapter may translate platform-specific I2C mechanics into the core transport contract, but must not redefine protocol semantics.

Current layout:

- `adapters/linux/`: Linux adapter implementation and notes
- `include/ezo_i2c/adapters/`: public adapter-facing headers
- `src/arduino/`: Arduino adapter implementation, kept in the library source set for Arduino-focused tooling such as PlatformIO
