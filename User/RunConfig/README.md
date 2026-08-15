# Linux AutoAim Run Config

这个目录放 `xrobot` 运行配置。仓库只保留一份生成后的
`User/xrobot_main.hpp`，切换配置后需要重新生成。

## Files

- `hik.yaml`：实机 Hik 相机配置，使用 `2x2` 下采样输出 `720x540`，触发目标为 `100Hz`。
- `vision_capture.yaml`：实机同步采集和标定数据配置，只实例化相机、同步、
  SharedTopic 收发和 VisionCapture，不实例化检测、跟踪和 Aimer。
- `capturefile.yaml`：离线文件配置，保持录像原始 `1440x1080` 几何。
- `camera_intrinsic_calibration.yaml`：原生 `1440x1080` 内参标定，记录原始 IMU，
  使用 GShang 标定板的纯视觉筛样，但不使用 IMU、跨时钟时间差或 PnP 拒绝图像。
- `camera_extrinsic_calibration.yaml`：原生 `1440x1080` 外参（手眼）数据采集，
  使用当前 K/D 做 PnP，并按 CameraBase 固定的 `m/s^2` IMU 契约判稳；只采集数据，
  不运行手眼求解器。

## Recording

需要同步图像、IMU、相机内参和标定板检测结果时使用 `vision_capture.yaml`。
默认输出：

```text
runs/vision_capture/hik_capture/
  camera_info.txt
  samples.csv
  images/
```

两份原生标定配置的采集会话分别写入 `runs/calibration/intrinsic/` 和
`runs/calibration/handeye/`。会话名留空，由 VisionCapture 建立时间戳目录。
内参求解结果单独写入 `runs/camera_calib/`；应检查其中的质量报告，并把通过质量门槛的
`camera_info_snippet.txt` K/D 更新到外参 YAML 后再采集手眼数据。采集会话内的
`camera_calibration.txt` 只是启动时冻结的 K/D，不是本轮内参求解结果。
若质量门槛未通过，本轮内参标定会终止并保留视角与质量报告；重启程序后再开始新会话。
浏览器预览地址分别为 `/stream/intrinsic_calibration`（端口 `8084`）和
`/stream/handeye_dataset`（端口 `8085`）。全幅外触发初值为 `20Hz`；USB 带宽、
相机全幅读出和可持续触发频率仍需实机确认。

## Generate

```bash
python3 -m xrobot.GenerateMain --output User/xrobot_main.hpp
python3 -m xrobot.GenerateMain --config User/RunConfig/hik.yaml --output User/xrobot_main.hpp
python3 -m xrobot.GenerateMain --config User/RunConfig/vision_capture.yaml --output User/xrobot_main.hpp
python3 -m xrobot.GenerateMain --config User/RunConfig/capturefile.yaml --output User/xrobot_main.hpp
python3 -m xrobot.GenerateMain --config User/RunConfig/camera_intrinsic_calibration.yaml --output User/xrobot_main.hpp
python3 -m xrobot.GenerateMain --config User/RunConfig/camera_extrinsic_calibration.yaml --output User/xrobot_main.hpp
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
