# BSP Linux AutoAim

Linux 实物自瞄 BSP，基于 `libxr` / `xrobot` 组织工程。

## Layout

```text
Modules/                  模块目录
User/                     用户配置和生成入口
User/RunConfig/           可选运行配置
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
- `User/RunConfig/hik_record.yaml`：实机录制专用入口，只实例化相机、同步和
  SharedTopic 收发，不实例化检测和跟踪模块，采集几何同样为 `720x540`。
  它只开启 CameraFrameSync 同步 IMU 记录，不再使用 CameraBase 侧图像内录。
- `User/RunConfig/vision_capture.yaml`：实机同步采集和标定数据入口，实例化相机、
  同步、SharedTopic 收发和 VisionCapture，不实例化检测、跟踪和 Aimer。同步图像、
  IMU、相机内参和 ArUco 检测预览写到 `runs/vision_capture/hik_capture/`。
- `User/RunConfig/capturefile.yaml`：使用内录文件验证视觉链路，不依赖 Hik 相机和 C 板，
  保持录像原始 `1440x1080` 几何。

## Generate

默认配置：

```bash
python3 -m xrobot.GenerateMain --output User/xrobot_main.hpp
```

指定运行配置：

```bash
python3 -m xrobot.GenerateMain --config User/RunConfig/hik.yaml --output User/xrobot_main.hpp
python3 -m xrobot.GenerateMain --config User/RunConfig/hik_record.yaml --output User/xrobot_main.hpp
python3 -m xrobot.GenerateMain --config User/RunConfig/vision_capture.yaml --output User/xrobot_main.hpp
python3 -m xrobot.GenerateMain --config User/RunConfig/capturefile.yaml --output User/xrobot_main.hpp
```

`User/xrobot_main.hpp` 和 `User/xrobot_constexpr.hpp` 是生成文件。

## Build

```bash
cmake -S . -B build/debug -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build/debug --target rm_auto_aim -j$(nproc)
```

## Run

```bash
./build/debug/rm_auto_aim
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
