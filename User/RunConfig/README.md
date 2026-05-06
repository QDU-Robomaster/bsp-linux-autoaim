# Linux AutoAim Run Config

这个目录放 `xrobot` 运行配置。仓库只保留一份生成后的
`User/xrobot_main.hpp`，切换配置后需要重新生成。

## Files

- `hik.yaml`：实机 Hik 相机配置。
- `hik_record.yaml`：实机 Hik 相机录制专用配置，只实例化相机、同步和
  SharedTopic 收发，不实例化检测和跟踪模块。
- `capturefile.yaml`：离线文件配置。

## Recording

`hik_record.yaml` 写出的 JPEG 内录包使用同一个 stem：

```text
runs/camera_record/<时间>_<相机名>/
  <stem>_frames.bin
  <stem>_frames.csv
  <stem>_camera_info.yaml
  <stem>_sync.csv
  <stem>_imu.csv
```

运行中先写到同级 `.tmp` 目录，并保留 `.recording` 恢复标记；如果比赛中直接拔电池，
下次启动会先整理上次未完成的包。回放时把 `CaptureFileCamera.runtime.file_path`
指到 `*_frames.bin`，`frame_csv_path` 指到 `*_frames.csv`，`imu_csv_path` 指到
`*_imu.csv`。

## Generate

```bash
python3 -m xrobot.GenerateMain --output User/xrobot_main.hpp
python3 -m xrobot.GenerateMain --config User/RunConfig/hik.yaml --output User/xrobot_main.hpp
python3 -m xrobot.GenerateMain --config User/RunConfig/hik_record.yaml --output User/xrobot_main.hpp
python3 -m xrobot.GenerateMain --config User/RunConfig/capturefile.yaml --output User/xrobot_main.hpp
```

不带 `--config` 时使用 `User/xrobot.yaml`。
