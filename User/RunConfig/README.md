# Linux autoaim xrobot presets

This directory follows the `bsp-dev-c` preset pattern: keep one generated
`User/xrobot_main.hpp`, and choose the module graph by passing a YAML config to
`xrobot_gen_main`.

## Presets

- `capturefile.yaml`
  - Source: `CaptureFileCamera`
  - Sync: set by YAML to `CameraFrameSync::LATEST_IMU`
  - Topics: `capturefile_image`, `capturefile_imu`, and raw IMU under
    `capturefile_camera_*`
  - Purpose: offline validation with a cleaned capture package:
    `/home/xiao/data/camera_internal_recording_20260428/damo_clean.avi` and
    `/home/xiao/data/camera_internal_recording_20260428/damo_imu.csv`
  - This preset intentionally contains only the production camera/sync/detector/tracker graph.

- `hik.yaml`
  - Source: `HikCamera`
  - Sync: set by YAML to `CameraFrameSync::RAW_PROBE`
  - Purpose: real camera with external hardware trigger and real raw IMU topics
  - Runtime requirement: the platform must provide `camera_gyro`, `camera_accl`,
    `camera_quat`, and a hardware-side responder for `sensor_sync_cmd`
  - Module requirement: current `HikCamera` master contains the CameraBase producer
    and timestamp diagnostics used by this preset.

## Generate

```bash
xrobot_gen_main --config User/RunConfig/capturefile.yaml --output User/xrobot_main.hpp
xrobot_gen_main --config User/RunConfig/hik.yaml --output User/xrobot_main.hpp
```
