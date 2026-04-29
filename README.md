# BSP Linux AutoAim

基于 `libxr` / `xrobot` 的 Linux 实物自瞄主仓。

当前主线只维护这一条核心链路：

```text
Camera -> CameraFrameSync -> ArmorDetector -> ArmorTracker
```

## Layout

```text
Modules/                 模块依赖清单
User/main.cpp            程序入口，初始化 LibXR 后调用 XRobotMain
User/xrobot_main.hpp     当前默认生成结果，默认使用 hik preset
User/xrobot_constexpr.hpp 当前默认生成常量
User/RunConfig/          可选择的 xrobot 装配 preset
libxr/                   框架 submodule
```

## Presets

- `User/RunConfig/hik.yaml`
  - 实机 Hik 相机入口
  - `CameraFrameSync` mode: `RAW_PROBE`
  - 默认签入的 `xrobot_main.hpp` 就是从这个 preset 生成
  - 需要真实 Hik 相机、硬件触发、以及板端发布 `camera_gyro/camera_accl/camera_quat`

- `User/RunConfig/capturefile.yaml`
  - 使用内录文件验证视觉链路
  - `CameraFrameSync` mode: `LATEST_IMU`
  - 使用独立的 `capturefile_*` topic 名，避免和实机入口冲突
  - 数据文件：`/home/xiao/data/camera_internal_recording_20260428/damo.avi`
  - 用于无 Hik 相机时跑通 sync/detector/tracker

## Generate

```bash
xrobot_gen_main --config User/RunConfig/hik.yaml --output User/xrobot_main.hpp
xrobot_gen_main --config User/RunConfig/capturefile.yaml --output User/xrobot_main.hpp
```

`xrobot_main.hpp` 和 `xrobot_constexpr.hpp` 是生成文件。改 preset 后重新生成，再编译。

## Build

```bash
git submodule update --init --recursive
xrobot_setup
xrobot_gen_main --config User/RunConfig/hik.yaml --output User/xrobot_main.hpp
cmake -S . -B build -G Ninja -DOpenVINO_DIR=/opt/intel/openvino_2025.4.0/runtime/cmake
cmake --build build --target rm_auto_aim
```

CI 会分别生成并构建 `capturefile.yaml` 和 `hik.yaml`。

## Run

```bash
./build/rm_auto_aim
```
