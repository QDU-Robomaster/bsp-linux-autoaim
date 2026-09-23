# BSP Linux AutoAim

Linux 实物自瞄 BSP，基于 `libxr` / `xrobot` 组织工程。

## Layout

```text
Modules/                  模块目录
User/                     实机入口、共用初始化和运行配置常量
User/RunConfig/           实机可选运行配置
User/Replay/              内录回放入口和配置
libxr/                    libxr submodule
CMakePresets.json         命令行 CMake preset
.vscode/                  VS Code Remote SSH 配置
```

## Prepare

模块和 submodule 由使用者按项目约定初始化。开始构建前确认这些目录已经存在：

```text
libxr/
Modules/
```

如果 OpenVINO 不在 CMake 默认搜索路径里，在本机环境中设置 `OpenVINO_DIR`
或 `CMAKE_PREFIX_PATH`。

## Presets

- `User/RunConfig/hik.yaml`：实机 Hik 相机入口，使用硬件触发和真实 IMU topic。
  Hik 使用 `2x2` 下采样输出 `720x540`，触发目标为 `100Hz`。
  手眼外参写在 `ArmorTracker.cfg.extrinsic.camera_to_body`，表示从 OpenCV
  相机系到公开本体系 `B` 的变换。
- `User/RunConfig/vision_capture.yaml`：实机同步采集和标定数据入口，实例化相机、
  同步、SharedTopic 收发和 VisionCapture，不实例化检测、跟踪和 Aimer。同步图像、
  IMU、相机内参和 ArUco 检测预览写到 `runs/vision_capture/hik_capture/`。
- `User/Replay/xrobot.yaml`：回放可执行文件 `rm_auto_aim_replay` 的配置，使用内录文件
  验证视觉链路，不依赖 Hik 相机和 C 板，保持录像原始 `1440x1080` 几何。

实机配置都连接 DevC USB，由 `rm_auto_aim` 构建；回放不打开 DevC，由
`rm_auto_aim_replay` 构建。两个可执行文件各自包含同目录下生成的 `xrobot_main.hpp`。
各配置使用的常量在 `User/run_config.hpp`，按配置分命名空间，YAML 直接引用自己的命名空间。

## Generate

实机默认配置和指定运行配置：

```bash
python3 -m xrobot.GenerateMain --config User/xrobot.yaml --output User/xrobot_main.hpp --register-source User/main.cpp
python3 -m xrobot.GenerateMain --config User/RunConfig/hik.yaml --output User/xrobot_main.hpp --register-source User/main.cpp
python3 -m xrobot.GenerateMain --config User/RunConfig/vision_capture.yaml --output User/xrobot_main.hpp --register-source User/main.cpp
```

回放：

```bash
python3 -m xrobot.GenerateMain --config User/Replay/xrobot.yaml --output User/Replay/xrobot_main.hpp --register-source User/Replay/main.cpp
```

`User/xrobot_main.hpp` 和 `User/Replay/xrobot_main.hpp` 是生成文件；`User/run_config.hpp`
手工维护。

## Build

```bash
cmake -S . -B build/debug -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build/debug --target rm_auto_aim rm_auto_aim_replay -j$(nproc)
```

## Run

```bash
./build/debug/rm_auto_aim          # 实机
./build/debug/rm_auto_aim_replay   # 内录回放
```

## VS Code

Linux BSP 预期在 Remote SSH 窗口里使用，不需要 Docker / Dev Container。

推荐扩展：

- `ms-vscode.cmake-tools`
- `llvm-vs-code-extensions.vscode-clangd`
- `webfreak.debug`
- `xrobot.xrobot`

常用入口：

- `CMake: Select a Kit`
- `Tasks: Run Task` -> `Build: capturefile debug`
- `Tasks: Run Task` -> `Build: hik debug`
- `Run and Debug` -> `Linux: Debug capturefile replay`
- `Run and Debug` -> `Linux: Debug Hik hardware`
