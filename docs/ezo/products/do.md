# EZO-DO

> Notice: This page is an original repo-authored summary informed by Atlas Scientific documentation. It is not an official Atlas Scientific manual.

## Purpose

This page captures the repo-relevant behavior of the EZO dissolved-oxygen family.

## Product Summary

- default transport mode is UART
- default I2C address is `97` (`0x61`)
- typical one-shot read timing is about `600 ms`
- supports one-point and two-point calibration
- supports temperature, salinity, and atmospheric-pressure compensation
- can expose dissolved oxygen in more than one output form

## Measurement Model

DO is not just a scalar sensor with a single correction term.

The product can report dissolved oxygen as:

- mg/L
- percent saturation

The enabled output set affects payload shape, so callers should not assume one fixed-field response unless device configuration is already controlled by the application.

## Shared Command Families Present

The DO family includes the common acquisition, calibration, identity, control, and protocol-switching families, plus calibration export and import.

## Product-Specific Features

### Compensation Surface

DO has the broadest compensation surface among the current products:

- temperature
- salinity
- atmospheric pressure

That is why the current typed helper layer exposes these settings explicitly, because they shape both calibration practice and measurement interpretation.

### Output Selection

The product can enable or disable mg/L and percent-saturation outputs. As with EC and HUM, payload parsing belongs in a product-aware layer.

The documented output-config query reply uses the alternate vendor form `?,O,...` rather than the more common `?Prefix,...` shape. The typed DO parser accepts that response directly.

### Pressure Compensation Query Shape

The atmospheric-pressure query is another vendor oddball: the documented reply uses `?,P,...`. That shape differs from the simpler `?T,...` and `?S,...` query families, so the DO helper treats it as product-specific parsing instead of relying on the generic shared query helper.

## Timing Notes

The generic repo read hint is conservative for normal DO reads. Calibration and compensation changes should still be treated as command-specific operations rather than inferred from the measurement timing alone.

## Code Implications

The current typed DO module now owns:

- typed parsing for configured outputs
- explicit compensation commands
- calibration query/set/clear helpers

Application code should still treat DO payload shape as configuration-dependent unless output configuration is controlled explicitly.
