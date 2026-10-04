# bsp-linux-autoaim

Linux 实机自瞄 BSP / Linux autoaim BSP for the real robot

## 1. 板子与平台 / Board and Platform

运行平台是带 Hik 工业相机的 Linux 主机，通过 USB 与 DevC 通信（VID `16d0`、PID `1492`，串口波特率 115200）。程序由 LibXR 和 XRobot 静态装配，入口源文件为 `User/main.cpp`，它注册 `ramfs` 与 `devc_usb` 后调用 `XROBOT_MAIN()`；平台初始化、终端线程和文件日志在 `User/bsp_common.hpp` 中。装甲板检测的模型由 `ArmorDetector` 的 `network.model` 决定：默认配置、`hik.yaml` 和 `sentry.yaml` 均为 `ArmorDetectorModel::INT16_HEAD_L`，使用 HailoRT 后端；`OPENVINO_640X512` 使用 OpenVINO。CMake 未找到 HailoRT 时，`INT16_HEAD_L` 在初始化时报错。HailoRT 或 OpenVINO 不在 CMake 默认搜索路径时，设置 `HailoRT_DIR`、`OpenVINO_DIR` 或 `CMAKE_PREFIX_PATH`。

`SharedTopic` 按名称查找收到的 Topic，而 `ArmorDetector` 与 `Aimer` 需要带类型的 `robot_game_ref`。因此 `User/main.cpp` 在 `XROBOT_MAIN()` 之前用 `RefereeTypes::RobotGameRefereePack` 创建该 Topic，`Modules/modules.yaml` 因此列出 `QDU-Robomaster/Referee`。

The platform is a Linux host with a Hik industrial camera, connected to the DevC over USB (VID `16d0`, PID `1492`, 115200 baud). The program is statically assembled with LibXR and XRobot. The entry source `User/main.cpp` registers `ramfs` and `devc_usb`, then calls `XROBOT_MAIN()`; platform initialization, the terminal thread and the file log are in `User/bsp_common.hpp`. The detection model is set by `network.model` of `ArmorDetector`: the default configuration, `hik.yaml` and `sentry.yaml` use `ArmorDetectorModel::INT16_HEAD_L`, which runs on the HailoRT backend, and `OPENVINO_640X512` uses OpenVINO. Without HailoRT found by CMake, `INT16_HEAD_L` fails at initialization. When HailoRT or OpenVINO is not in the default CMake search path, `HailoRT_DIR`, `OpenVINO_DIR` or `CMAKE_PREFIX_PATH` points to it.

`SharedTopic` looks up received Topics by name, while `ArmorDetector` and `Aimer` need the typed `robot_game_ref`. `User/main.cpp` therefore creates this Topic with `RefereeTypes::RobotGameRefereePack` before `XROBOT_MAIN()`, and `Modules/modules.yaml` lists `QDU-Robomaster/Referee` for the same reason.

## 2. 配置一览 / Configurations

| 配置 | 产品或用途 |
| --- | --- |
| `User/xrobot.yaml` | 默认配置：Hik 相机、硬件触发、DevC 回传的 IMU，完整自瞄链路 |
| `User/RunConfig/hik.yaml` | 实机 Hik 配置，链路与默认配置相同，`Aimer` 的 `yaw_offset`、`roll_offset`、`default_bullet_speed` 取值不同 |
| `User/RunConfig/sentry.yaml` | 哨兵配置，使用独立标定 |
| `User/RunConfig/vision_capture.yaml` | 同步采集与标定数据：实例化相机、同步、SharedTopic 收发和 VisionCapture，输出到 `runs/vision_capture/hik_capture/` |

| Configuration | Product or purpose |
| --- | --- |
| `User/xrobot.yaml` | Default: Hik camera, hardware trigger, IMU from the DevC, full autoaim chain |
| `User/RunConfig/hik.yaml` | Real-robot Hik configuration with the same chain as the default; `yaw_offset`, `roll_offset` and `default_bullet_speed` of `Aimer` differ |
| `User/RunConfig/sentry.yaml` | Sentry configuration with its own calibration |
| `User/RunConfig/vision_capture.yaml` | Synchronized capture and calibration data: camera, sync, SharedTopic transfer and VisionCapture; output goes to `runs/vision_capture/hik_capture/` |

Hik 使用 `2x2` 下采样，输出 `720x540`。手眼外参为 `ArmorTracker` 配置中的 `extrinsic.camera_mount_to_body`（相机安装坐标系到本体系 `B`，旋转为 wxyz 四元数，平移单位 m）。各配置用到的常量写在配置自己的 `constexprs` 段，生成到 `AutoAimRunConfig` 命名空间。

The Hik camera uses `2x2` decimation and outputs `720x540`. The hand-eye extrinsic is `extrinsic.camera_mount_to_body` in the `ArmorTracker` configuration (camera mount frame to body frame `B`, rotation as a wxyz quaternion, translation in meters). The constants of each configuration are in its own `constexprs` section and are generated into the `AutoAimRunConfig` namespace.

配置文件的格式见 [XRobot 文档](https://xrobot.work/docs/proj_man/proj-man-config)。

The configuration format is described in the [XRobot documentation](https://xrobot.work/docs/proj_man/proj-man-config).

## 3. 构建 / Build

环境：C++20 编译器、CMake 3.21 或更高、Ninja、OpenCV 4（需要 contrib 中的 `aruco`）、xrobot（版本与 `Modules/modules.yaml` 的 `xrobot:` 一致，当前为 1.0.0），以及上文所述的 HailoRT 或 OpenVINO。海康 MVS SDK 安装在 `/opt/MVS` 时使用该 SDK，否则使用 HikCamera 模块自带的 SDK。

```bash
git submodule update --init --recursive
pip install xrobot==1.0.0
xrobot setup
xrobot gen -c User/RunConfig/hik.yaml
cmake --preset debug
cmake --build --preset debug --target rm_auto_aim
```

`xrobot==1.0.0` 与 `Modules/modules.yaml` 的 `xrobot:` 字段一致，LibXR 由 `libxr/` 子模块固定到具体提交。`xrobot setup` 拉取模块、检查所有配置并生成 `User/xrobot_main.hpp`；`xrobot gen -c <配置>` 选择要构建的配置，省略 `-c` 时使用当前选中的配置（已生成的 `User/xrobot_main.hpp` 对应的配置），尚未生成时为 `User/xrobot.yaml`。构建时 LibXR 检查配置、lock（`xrobot.lock`）、模块头文件和入口源文件中的注册在生成 `User/xrobot_main.hpp` 之后是否发生变化，变化时构建失败并提示对应的 `xrobot gen -c <配置>` 命令。CMake 预设有 `debug`、`relWithDebInfo` 和 `release`。

Environment: a C++20 compiler, CMake 3.21 or newer, Ninja, OpenCV 4 (with `aruco` from contrib), xrobot (the version equals the `xrobot:` field of `Modules/modules.yaml`, currently 1.0.0), and HailoRT or OpenVINO as described above. The Hikvision MVS SDK in `/opt/MVS` is used when it is installed, otherwise the SDK bundled with the HikCamera Module.

`xrobot==1.0.0` matches the `xrobot:` field of `Modules/modules.yaml`, and the `libxr/` submodule pins LibXR to a commit. `xrobot setup` fetches the Modules, checks every configuration and generates `User/xrobot_main.hpp`; `xrobot gen -c <configuration>` selects the configuration to build, and without `-c` it uses the selected configuration (the one the generated `User/xrobot_main.hpp` was made from), or `User/xrobot.yaml` before anything is generated. During the build LibXR checks whether the configuration, the lock (`xrobot.lock`), the Module headers and the registrations in the entry source changed after `User/xrobot_main.hpp` was generated; on a change the build fails and prints the matching `xrobot gen -c <configuration>` command. The CMake presets are `debug`, `relWithDebInfo` and `release`.

## 4. 烧录与运行 / Flash and Run

```bash
./build/debug/rm_auto_aim
```

可执行文件为 `build/<preset>/rm_auto_aim`，在仓库根目录运行。同一时间只能运行一个自瞄进程，终端日志同时写入以启动时间命名的 `YYYYMMDD_HHMMSS.log`。`.vscode/` 提供 Remote-SSH 窗口下的任务（`XRobot: setup`、`XRobot: select <配置>`、`Build: <配置> debug`）和与每份配置对应的 GDB 调试配置。

The executable is `build/<preset>/rm_auto_aim`, run from the repository root. One autoaim process runs at a time, and the log is also written to a `YYYYMMDD_HHMMSS.log` file named after the start time. `.vscode/` provides tasks for a Remote-SSH window (`XRobot: setup`, `XRobot: select <configuration>`, `Build: <configuration> debug`) and a GDB launch entry for each configuration.
