# Linux autoaim xrobot 预设

这个目录沿用 `bsp-dev-c` 的预设模式：仓库只保留一份生成后的
`User/xrobot_main.hpp`，不同链路通过 `User/RunConfig/*.yaml` 生成。

## Presets

- `capturefile.yaml`
  - 输入源：`CaptureFileCamera`
  - 同步模式：`CameraFrameSync::LATEST_IMU`
  - Topic：`capturefile_image`、`capturefile_imu`，原始 IMU 使用
    `capturefile_camera_*`
  - 用途：使用清洗后的内录包做离线验证：
    `/home/xiao/data/camera_internal_recording_20260428/damo_clean.avi` and
    `/home/xiao/data/camera_internal_recording_20260428/damo_imu.csv`
  - `VisionPreview` 默认关闭；需要录像或实时预览时只改它的运行时配置。

- `hik.yaml`
  - 输入源：`HikCamera`
  - 同步模式：`CameraFrameSync::RAW_PROBE`
  - 用途：实机 Hik 相机、外部硬件触发和真实原始 IMU topic。
  - 运行要求：平台必须提供 `camera_gyro`、`camera_accl`、`camera_quat`，
    并在硬件侧响应 `sensor_sync_cmd`。

## 预览配置

两个 preset 都实例化 `VisionPreview`，但默认关闭：

- `enabled`：总开关。
- `record_raw`：原始视频和 topic TSV 落盘。
- `realtime_preview`：实时窗口预览。
- `overlay.detector`：绘制 detector 结果。
- `overlay.tracker`：绘制 tracker 结果。
- `overlay.candidate_debug`：显示候选调试统计。

## Generate

```bash
xrobot_gen_main --config User/RunConfig/capturefile.yaml --output User/xrobot_main.hpp
xrobot_gen_main --config User/RunConfig/hik.yaml --output User/xrobot_main.hpp
```
