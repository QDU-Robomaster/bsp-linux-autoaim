# 配置 / Configurations

本目录的每个 YAML 是一份配置，`User/xrobot.yaml` 是默认配置。

- `hik.yaml`：实机 Hik 配置，`2x2` 下采样输出 `720x540`，触发目标 `100Hz`。链路与 `User/xrobot.yaml` 相同，`Aimer` 的 `yaw_offset`、`roll_offset`、`default_bullet_speed` 取值不同。
- `sentry.yaml`：哨兵配置，使用独立标定。
- `vision_capture.yaml`：同步采集和标定数据，实例化相机、同步、SharedTopic 收发和 VisionCapture；输出到 `runs/vision_capture/hik_capture/`（`camera_info.txt`、`samples.csv`、`images/`）。

选择配置：

```bash
xrobot gen -c User/RunConfig/hik.yaml
```

所有配置都用 `FrameLayout` 描述像素缓冲区，`CameraCalibration` 保存原生传感器坐标下的内参与畸变。裁判 Topic 为 `robot_game_ref`，由 `User/main.cpp` 创建（见仓库 README）。

`ArmorDetector` 的 `network.model` 取值（`ArmorDetectorModel::` 前缀）：`INT8_HEAD_L`、`INT8_GRID_L`、`INT16_HEAD_L`、`INT8_HEAD`、`INT8_GRID`、`INT16_HEAD`、`INT16_FAST_L`、`INT16_FAST`、`OPENVINO_640X512`，例如：

```yaml
network:
  model: ArmorDetectorModel::INT8_GRID_L
```

Each YAML in this directory is a configuration; `User/xrobot.yaml` is the default.

- `hik.yaml`: real-robot Hik configuration, `2x2` decimation with `720x540` output and a `100Hz` trigger target. The chain equals `User/xrobot.yaml`; `yaw_offset`, `roll_offset` and `default_bullet_speed` of `Aimer` differ.
- `sentry.yaml`: sentry configuration with its own calibration.
- `vision_capture.yaml`: synchronized capture of calibration data, instantiating the camera, sync, SharedTopic transfer and VisionCapture; output goes to `runs/vision_capture/hik_capture/` (`camera_info.txt`, `samples.csv`, `images/`).

Select a configuration:

```bash
xrobot gen -c User/RunConfig/hik.yaml
```

All configurations describe the pixel buffer with `FrameLayout`, and `CameraCalibration` holds the intrinsics and distortion in native sensor coordinates. The referee Topic is `robot_game_ref`, created by `User/main.cpp` (see the repository README).

The values of `network.model` in `ArmorDetector` (prefix `ArmorDetectorModel::`) are `INT8_HEAD_L`, `INT8_GRID_L`, `INT16_HEAD_L`, `INT8_HEAD`, `INT8_GRID`, `INT16_HEAD`, `INT16_FAST_L`, `INT16_FAST` and `OPENVINO_640X512`, for example:

```yaml
network:
  model: ArmorDetectorModel::INT8_GRID_L
```
