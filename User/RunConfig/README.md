# Linux autoaim xrobot presets

This directory follows the `bsp-dev-c` preset pattern: keep one generated
`User/xrobot_main.hpp`, and choose the module graph by passing a YAML config to
`xrobot_gen_main`.

## Presets

- `capturefile.yaml`
  - Source: `CaptureFileCamera`
  - Sync: `CameraFrameSync::LATEST_IMU`
  - Purpose: offline validation with `/home/xiao/data/camera_internal_recording_20260428/damo.avi`
  - This preset intentionally contains only the production camera/sync/detector/tracker graph.

- `hik.yaml`
  - Source: `HikCamera`
  - Sync: `CameraFrameSync::RAW_PROBE` default
  - Purpose: real camera with external hardware trigger and real raw IMU topics
  - Runtime requirement: the platform must provide `camera_gyro`, `camera_accl`,
    `camera_quat`, and a hardware-side responder for `sensor_sync_cmd`
  - Review branch requirement: use `HikCamera` branch
    `codex/hik-camerabase-20260429` until the module update is merged

## Generate

```bash
xrobot_gen_main --config User/RunConfig/capturefile.yaml --output User/xrobot_main.hpp
xrobot_gen_main --config User/RunConfig/hik.yaml --output User/xrobot_main.hpp
```

Validation scripts may temporarily add `AutoAimRuntimeProbe` to count topics, but that
probe is not part of either preset.
