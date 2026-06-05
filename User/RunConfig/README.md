# Linux AutoAim Run Config

这个目录放 `xrobot` 运行配置。仓库只保留一份生成后的
`User/xrobot_main.hpp`，切换配置后需要重新生成。

## Files

- `hik.yaml`：实机 Hik 相机配置，使用 `2x2` 下采样输出 `720x540`，触发目标为 `100Hz`。
- `vision_capture.yaml`：实机同步采集和标定数据配置，只实例化相机、同步、
  SharedTopic 收发和 VisionCapture，不实例化检测、跟踪和 Aimer。
- `capturefile.yaml`：离线文件配置，保持录像原始 `1440x1080` 几何。

## Recording

需要同步图像、IMU、相机内参和标定板检测结果时使用 `vision_capture.yaml`。
默认输出：

```text
runs/vision_capture/hik_capture/
  camera_info.txt
  samples.csv
  images/
```

## Generate

```bash
python3 -m xrobot.GenerateMain --output User/xrobot_main.hpp
python3 -m xrobot.GenerateMain --config User/RunConfig/hik.yaml --output User/xrobot_main.hpp
python3 -m xrobot.GenerateMain --config User/RunConfig/vision_capture.yaml --output User/xrobot_main.hpp
python3 -m xrobot.GenerateMain --config User/RunConfig/capturefile.yaml --output User/xrobot_main.hpp
```

不带 `--config` 时使用 `User/xrobot.yaml`。

## Hailo Variants

当前 detector 已支持固定 `network.model` 枚举选择。当前支持的 `6` 个值是：

- `ArmorDetectorModel::INT8_HEAD_L`
- `ArmorDetectorModel::INT8_GRID_L`
- `ArmorDetectorModel::INT16_HEAD_L`
- `ArmorDetectorModel::INT8_HEAD`
- `ArmorDetectorModel::INT8_GRID`
- `ArmorDetectorModel::INT16_HEAD`

示例：

```yaml
network:
  model: {expr: ArmorDetectorModel::INT8_GRID_L}
```
