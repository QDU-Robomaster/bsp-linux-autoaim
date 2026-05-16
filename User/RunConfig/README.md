# Linux AutoAim Run Config

这个目录放 `xrobot` 运行配置。仓库只保留一份生成后的
`User/xrobot_main.hpp`，切换配置后需要重新生成。

## Files

- `hik.yaml`：实机 Hik 相机配置，使用 `2x2` 下采样输出 `720x540`，触发目标为 `100Hz`。
- `hik_record.yaml`：实机 Hik 相机录制专用配置，只实例化相机、同步和
  SharedTopic 收发，不实例化检测和跟踪模块，采集几何同样为 `720x540`。
- `vision_capture.yaml`：实机同步采集和标定数据配置，只实例化相机、同步、
  SharedTopic 收发和 VisionCapture，不实例化检测、跟踪和 Aimer。
- `capturefile.yaml`：离线文件配置，保持录像原始 `1440x1080` 几何。

## Recording

`hik_record.yaml` 只保留 CameraFrameSync 同步记录：

```text
runs/camera_sync/<时间>_<相机名>/
  sync.csv
  imu.csv
```

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
python3 -m xrobot.GenerateMain --config User/RunConfig/hik_record.yaml --output User/xrobot_main.hpp
python3 -m xrobot.GenerateMain --config User/RunConfig/vision_capture.yaml --output User/xrobot_main.hpp
python3 -m xrobot.GenerateMain --config User/RunConfig/capturefile.yaml --output User/xrobot_main.hpp
```

不带 `--config` 时使用 `User/xrobot.yaml`。
