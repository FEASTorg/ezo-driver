# Arduino I2C D.O. Calibration

This sketch stages dissolved-oxygen calibration with explicit compensation-state inspection, preview reads, and post-apply verification.

## Before You Start

- The vendor guidance is calibrate first, compensate later.
- Before calibration, verify the compensation state is still at the vendor defaults: `20.0 C`, `0.0 ppt`, and `101.0 kPa`.
- The sketch defaults to `CALIBRATION_STEP=STEP_STATUS`, so the first upload is inspect-only unless you change it.

## Run It

- Start with `STEP_STATUS` and `APPLY_CHANGES=0` to inspect the current compensation state and current calibration level.
- Set `CALIBRATION_STEP=STEP_ZERO` and upload once with `APPLY_CHANGES=0` to watch the preview readings trend toward zero in the zero-oxygen solution.
- Re-upload `STEP_ZERO` with `APPLY_CHANGES=1` to commit the low point.
- After the low point, expose the probe to air, wait for the readings to stabilize, then repeat the same two-pass pattern with `STEP_HIGH`.
- The vendor says the low point comes first. A new probe may reach zero in minutes, but a probe with fresh electrolyte can take much longer.

## Check Result

- Re-run `STEP_STATUS` after calibration and confirm the new calibration level.
- At the vendor default compensation values, the post-high-point reading should settle around `9.09` to `9.1x mg/L` in air.
