# Arduino I2C RTD Calibration

This sketch stages RTD single-point offset calibration against a known reference temperature.

## Before You Start

- RTD uses single-point offset calibration only.
- Use a trusted stable reference temperature. A dry-block calibrator or another controlled reference is the vendor-aligned approach.
- The vendor guidance is explicit: never do a blind calibration. Wait for the reading to stabilize first.
- The sketch defaults to `CALIBRATION_STEP=STEP_STATUS`, so the first upload is inspect-only unless you change it.

## Run It

- Start with `STEP_STATUS` and `APPLY_CHANGES=0` to inspect the current scale and whether the circuit already reports calibrated state.
- Use `../rtd_workflow/rtd_workflow.ino` or another live read path to watch the RTD reading settle at the reference temperature before you commit calibration.
- Set `CALIBRATION_STEP=STEP_CALIBRATE` and `REFERENCE_TEMPERATURE_C` to the known reference value.
- Upload once with `APPLY_CHANGES=0`, then again with `APPLY_CHANGES=1` to commit. Use `STEP_CLEAR` with `APPLY_CHANGES=1` only when you intentionally want to remove calibration.

## Check Result

- Re-run `STEP_STATUS` and confirm the circuit now reports calibrated state.
- The correction is an offset: if the probe read `100.35 C` at a true `100.00 C`, calibrating to `100.00` removes that offset.
