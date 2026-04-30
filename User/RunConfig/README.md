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
  - `VisionPreview` 默认写原始视频、overlay 视频和 topic TSV 到
    `/tmp/autoaim_preview_capturefile`。

- `hik.yaml`
  - 输入源：`HikCamera`
  - 同步模式：`CameraFrameSync::RAW_PROBE`
  - 用途：实机 Hik 相机、外部硬件触发和真实原始 IMU topic。
  - 运行要求：平台必须提供 `camera_gyro`、`camera_accl`、`camera_quat`，
    并在硬件侧响应 `sensor_sync_cmd`。
  - `VisionPreview` 默认写原始视频、overlay 视频和 topic TSV 到
    `/tmp/autoaim_preview_hik`。

## 预览配置

两个 preset 都实例化并启用 `VisionPreview` 的原始数据落盘：

- `enabled`：总开关，当前两个 preset 默认开启。
- `record_raw`：原始视频、overlay 视频和 topic TSV 落盘，当前两个 preset 默认开启。
- `realtime_preview`：实时窗口预览，当前默认关闭，避免无显示后端时影响启动。
- `overlay.detector`：绘制 detector 结果。
- `overlay.tracker`：绘制 tracker 中心和当前匹配观测对应的 EKF 面。
- `overlay.candidate_debug`：显示候选调试统计。

落盘文件包括 `raw.avi`、`overlay.avi`、`detector.tsv`、`metrics.tsv`、
`target.tsv`、`ekf_points.tsv`、`candidate_debug.tsv` 和
`candidate_items.tsv`。EKF 模型补全的其它装甲板点不默认显示，需要在
VisionPreview 中显式打开 `overlay.model_faces`。

## Generate

```bash
xrobot_gen_main --config User/RunConfig/capturefile.yaml --output User/xrobot_main.hpp
xrobot_gen_main --config User/RunConfig/hik.yaml --output User/xrobot_main.hpp
```
