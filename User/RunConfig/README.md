# Linux AutoAim Run Config

这个目录放 `xrobot` 运行配置。仓库只保留一份生成后的
`User/xrobot_main.hpp`，切换配置后需要重新生成。

## Files

- `hik.yaml`：实机 Hik 相机配置，使用 `2x2` 下采样输出 `720x540`，触发目标为 `100Hz`。
- `vision_capture.yaml`：实机同步采集和标定数据配置，只实例化相机、同步、
  SharedTopic 收发和 VisionCapture，不实例化检测、跟踪和 Aimer。
- `capturefile.yaml`：离线文件配置，保持录像原始 `1440x1080` 几何。
- `sentry.yaml`：哨兵自由运行配置，使用其独立标定与 `sentry_ref` 裁判输入。

`User/xrobot.yaml` 是默认配置。上述配置均使用 `FrameLayout` 描述像素缓冲区，
`CameraCalibration` 保存原生传感器坐标下的内参与畸变。文件回放另传入固定的
`FrameGeometry`；下采样后的图像不再保存另一套缩放内参。

裁判话题名由 `RefereeTopicName` 统一指定：默认/hik 为 `robot_game_ref`，哨兵为
`sentry_ref`。BSP 在构造接收器前按 `RefereeTypes::RobotGameRefereePack` 创建该话题，
Detector 和 Aimer 直接订阅配置指定的输入。`capturefile` 保持 DevC USB 关闭。

使用 `cmake --preset release` 构建优化版本；调试选项由对应 CMake 构建类型决定。
CI 会生成并编译默认及全部四个 RunConfig。OpenVINO 探测是可选的，当前 Hailo
Detector 的实际推理仍需要对应 HailoRT、设备及模型。

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
