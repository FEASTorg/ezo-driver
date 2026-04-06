# Arduino I2C ORP Calibration

This sketch stages ORP single-point calibration with bounded preview reads.

## Before You Start

- Use a known ORP reference solution. `225 mV` is the common vendor-standard example and the sketch default.
- The sketch defaults to `CALIBRATION_STEP=STEP_STATUS`, so the first upload is inspect-only unless you change it.

## Run It

- Start with `STEP_STATUS` and `APPLY_CHANGES=0` to inspect whether the circuit already reports calibrated state.
- Set `CALIBRATION_STEP=STEP_CALIBRATE`, leave `REFERENCE_MV=225.0` unless you intentionally use a different certified solution, and upload once with `APPLY_CHANGES=0`.
- Confirm the preview readings are stable in the ORP reference solution.
- Re-upload with `APPLY_CHANGES=1` to commit the calibration, or use `STEP_CLEAR` with `APPLY_CHANGES=1` to remove calibration.

## Check Result

- Re-run `STEP_STATUS` and confirm the circuit now reports calibrated state.
- This is a single-point offset calibration, so there is no low/high sequence the way pH and D.O. use.
