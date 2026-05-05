# Linux autoaim xrobot 预设

这个目录沿用 `bsp-dev-c` 的预设模式：仓库只保留一份生成后的
`User/xrobot_main.hpp`，不同链路通过 `User/RunConfig/*.yaml` 生成。
`User/xrobot.yaml` 是不带 `--config` 调用 `xrobot_gen_main` 时使用的默认配置，
内容等价于 `hik.yaml`。

## Presets

- `capturefile.yaml`
  - 输入源：`CaptureFileCamera`
  - 同步模式：`CameraFrameSync::LATEST_IMU`
  - Topic：`capturefile_image`、`capturefile_imu`，原始 IMU 使用
    `capturefile_camera_*`
  - 用途：使用清洗后的内录包做离线验证：
    `/home/xiao/data/camera_internal_recording_20260428/damo_clean.avi` and
    `/home/xiao/data/camera_internal_recording_20260428/damo_imu.csv`
  - 算法链路：`Camera -> Sync -> Detector -> Tracker`。
  - 控制边界：内录只跑到 tracker 输出，不连接任何云台/发射执行器。
  - `EnableDevCUsb = false`，不实例化 C 板 USB UART。

- `hik.yaml`
  - 输入源：`HikCamera`
  - 同步模式：`CameraFrameSync::RAW_PROBE`
  - 算法链路：`Camera -> Sync -> Detector -> Tracker`。
  - 用途：实机 Hik 相机、外部硬件触发和真实原始 IMU topic。
  - 运行要求：平台必须提供 `gimbal_gyro`、`gimbal_accl`、`gimbal_quat`，
    并在硬件侧响应 `camera_sync_command`。
  - `EnableDevCUsb = true`，Linux 侧以别名 `DevC-USB` 注册 C 板 USB CDC UART。
  - `SharedTopic` 接收 `gimbal_gyro/gimbal_accl/gimbal_quat/camera_sync_result`，
    `SharedTopicClient` 发送 `camera_sync_command`。

## Tracker preset

模块源固定为：
`QDU-Robomaster/ArmorTracker@codex/tracker-algorithm-20260502`。


## Generate

```bash
xrobot_gen_main --output User/xrobot_main.hpp
xrobot_gen_main --config User/RunConfig/capturefile.yaml --output User/xrobot_main.hpp
xrobot_gen_main --config User/RunConfig/hik.yaml --output User/xrobot_main.hpp
```
