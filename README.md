# BSP Linux AutoAim

基于 `libxr` / `xrobot` 的 Linux 实物自瞄主仓。

当前主线已经统一到共享视觉核心：

- `ArmorDetector`
- `ArmorTracker`
- `Aimer`

Linux 主仓只负责把这套共享核心接到实物输入链：

- `HikCamera`
- `DemoReplay`
- `ReplayMetrics`

## Role

- `rm_auto_aim`
  - 实机入口
- `rm_auto_aim_demo`
  - 离线回放入口

## Layout

```text
Modules/   模块依赖清单
User/      xrobot 装配配置与生成入口
Video/     离线回放素材
libxr/     框架与底层组件
```

## Assembly

- `Modules/modules.yaml`
  - 决定需要拉取哪些模块仓库
- `User/xrobot.yaml`
  - 实机装配配置
- `User/xrobot_demo.yaml`
  - 离线回放装配配置
- `User/xrobot_main.hpp`
  - 由 `xrobot_gen_main` 生成，不手改
- `User/xrobot_main_demo.hpp`
  - 由 `xrobot_gen_main` 生成，不手改

## Core Topic Contract

共享视觉主线的数据流是：

1. 相机侧发布
   - `image_raw`
   - `camera_info`
2. `ArmorDetector` 输出
   - `armor_detector/armors_result`
   - `armor_detector/metrics`
3. `ArmorTracker` 输出
   - `tracker/info`
   - `tracker/metrics`
   - `tracker/target`
4. `Aimer` 输出
   - `tracker/target_eulr`
   - `tracker/send`
   - `aimer/metrics`

Linux 主仓在这条共享主线之外还会接入：

- 实机输入
  - `HikCamera`
- 离线输入
  - `DemoReplay`
- 离线评估
  - `ReplayMetrics`

## Build

```bash
git submodule update --init --recursive
xrobot_init_mod
xrobot_gen_main --output User/xrobot_main.hpp
xrobot_gen_main --config User/xrobot_demo.yaml --output User/xrobot_main_demo.hpp
cmake -S . -B build -G Ninja -DOpenVINO_DIR=/opt/intel/openvino_2025.4.0/runtime/cmake
cmake --build build -j$(nproc)
```

## Run

实机：

```bash
./build/rm_auto_aim
```

离线回放：

```bash
./build/rm_auto_aim_demo
```

## Preview

预览统一只由 YAML 控制，不走 CMake 开关。

- `armor_detector.cfg.debug.preview`
- `armor_tracker.cfg.debug.preview`

## Notes

- `DemoReplay` 和 `ReplayMetrics` 主要服务于离线验证链路
- `tracker.sh` 属于历史脚本，不是当前主程序入口
- 当前共享视觉核心已经回到各模块仓库的 `master`
