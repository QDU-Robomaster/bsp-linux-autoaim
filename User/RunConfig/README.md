# 配置 / Configurations

本目录中各份配置的用途和选择方法见[仓库 README](../../README.md)。`vision_capture.yaml` 的输出目录 `runs/vision_capture/hik_capture/` 中有 `camera_info.txt`、`samples.csv` 和 `images/`。

所有配置都用 `FrameLayout` 描述像素缓冲区，`CameraCalibration` 保存原生传感器坐标下的内参与畸变。

`ArmorDetector` 的 `network.model` 取值（`ArmorDetectorModel::` 前缀）：`INT8_HEAD_L`、`INT8_GRID_L`、`INT16_HEAD_L`、`INT8_HEAD`、`INT8_GRID`、`INT16_HEAD`、`INT16_FAST_L`、`INT16_FAST`、`OPENVINO_640X512`，例如：

```yaml
network:
  model: ArmorDetectorModel::INT8_GRID_L
```

The purpose of each configuration in this directory and how to select one are described in the [repository README](../../README.md). The output directory of `vision_capture.yaml`, `runs/vision_capture/hik_capture/`, holds `camera_info.txt`, `samples.csv` and `images/`.

All configurations describe the pixel buffer with `FrameLayout`, and `CameraCalibration` holds the intrinsics and distortion in native sensor coordinates.

The values of `network.model` in `ArmorDetector` (prefix `ArmorDetectorModel::`) are `INT8_HEAD_L`, `INT8_GRID_L`, `INT16_HEAD_L`, `INT8_HEAD`, `INT8_GRID`, `INT16_HEAD`, `INT16_FAST_L`, `INT16_FAST` and `OPENVINO_640X512`, for example:

```yaml
network:
  model: ArmorDetectorModel::INT8_GRID_L
```
