# Autoaim validation helpers

This directory keeps validation-only helpers out of production xrobot presets.

- `AutoAimRuntimeProbe/` counts camera, sync, detector, and tracker topics during
  CaptureFile validation.
- Do not add this probe to `User/RunConfig/capturefile.yaml` or `hik.yaml`.
- For validation, copy the probe into a temporary BSP build tree and generate a
  temporary `capturefile_probe.yaml` from `capturefile.yaml`.

The validated production-style presets are under `User/RunConfig/`.
