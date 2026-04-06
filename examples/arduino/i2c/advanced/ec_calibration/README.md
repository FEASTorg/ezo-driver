# Arduino I2C EC Calibration

This sketch stages EC calibration and shows the current temperature, probe-K, and calibration state before you apply changes.

## Before You Start

- Set `PROBE_K` to match the actual EC probe before calibrating. The default `1.0` is only correct for a K1.0 probe.
- Keep temperature compensation at the vendor default `25 C` during calibration. The vendor explicitly says not to change it while calibrating.
- Dry calibration is always first, even if the probe already reads `0.00`.
- The sketch defaults to `CALIBRATION_STEP=STEP_STATUS`, so the first upload is inspect-only unless you change it.

## Run It

- Start with `STEP_STATUS` and `APPLY_CHANGES=0` to inspect the current temperature compensation, probe K, and calibration state.
- Decide which calibration path you are doing:
- Two-point: `STEP_DRY` first, then `STEP_SINGLE` with `REFERENCE_US` set to your solution value.
- Three-point vendor-recommended path: `STEP_DRY`, then `STEP_LOW` with the low-point solution value, then `STEP_HIGH` with the high-point solution value.
- Use fresh solution in a cup, shake the probe to remove trapped air, and wait for the live EC reading to stabilize before the apply pass.
- This sketch is step-focused; if you want to watch the live conductivity settle before each apply, use `../ec_workflow/ec_workflow.ino` or another live read path first, then come back here to commit the step.
- Upload each step once with `APPLY_CHANGES=0`, then again with `APPLY_CHANGES=1` to commit it.

## Check Result

- Re-run `STEP_STATUS` after the final step and confirm the calibration level changed as expected.
- Vendor guidance says the low-point step may not visibly change the displayed reading right away, while the high-point step usually does.
