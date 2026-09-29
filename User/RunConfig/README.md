# 产品配置

本目录的每个 YAML 是一个产品配置，`User/xrobot.yaml` 是默认产品。

- `hik.yaml`：实机 Hik 相机，`2x2` 下采样输出 `720x540`，触发目标 `100Hz`。
- `sentry.yaml`：哨兵配置，使用独立标定。
- `vision_capture.yaml`：同步采集和标定数据，只实例化相机、同步、SharedTopic 收发和
  VisionCapture；输出到 `runs/vision_capture/hik_capture/`（`camera_info.txt`、
  `samples.csv`、`images/`）。

切换产品：

```bash
xrobot gen -c User/RunConfig/hik.yaml
```

所有配置都用 `FrameLayout` 描述像素缓冲区，`CameraCalibration` 保存原生传感器坐标下的内参与畸变。
裁判话题统一为 `robot_game_ref`，由 `User/main.cpp` 创建（见仓库 README）。

`ArmorDetector` 的 `network.model` 可选：`INT8_HEAD_L`、`INT8_GRID_L`、`INT16_HEAD_L`、
`INT8_HEAD`、`INT8_GRID`、`INT16_HEAD`（`ArmorDetectorModel::` 前缀），例如：

```yaml
network:
  model: 'ArmorDetectorModel::INT8_GRID_L'
```
