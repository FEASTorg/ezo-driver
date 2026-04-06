# Arduino I2C HUM Temperature Calibration

This sketch stages the HUM circuit's temperature calibration only. Humidity calibration remains at the vendor factory default.

## Before You Start

- Only use this if you actually need to correct the on-board temperature reading. The vendor position is that most humidity sensors do not need humidity calibration.
- Compare the circuit's temperature reading against a trusted ambient reference first, typically with `../hum_workflow/hum_workflow.ino`.
- The sketch defaults to `CALIBRATION_STEP=STEP_STATUS`, so the first upload is inspect-only unless you change it.

## Run It

- Start with `STEP_STATUS` and `APPLY_CHANGES=0` to inspect whether temperature calibration is already present.
- Set `REFERENCE_TEMPERATURE_C` to the trusted ambient reference temperature.
- Upload once with `APPLY_CHANGES=0`, then again with `APPLY_CHANGES=1` to commit. Use `STEP_CLEAR` with `APPLY_CHANGES=1` only when you intentionally want to remove the temperature calibration.

## Check Result

- Re-run `STEP_STATUS` and then `../hum_workflow/hum_workflow.ino` to confirm the reported air temperature now matches the trusted reference more closely.
- This sketch does not calibrate humidity itself; it only adjusts the temperature side that humidity calculations depend on.
