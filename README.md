# BSP Linux AutoAim

Linux 实物自瞄 BSP：Hik 相机 + DevC USB 链路，基于 LibXR / XRobot 静态装配。
内录回放是单独的 BSP（`bsp-linux-autoaim-replay`）。

## 目录

```text
Modules/modules.yaml     需要的模块（`xrobot:` 固定 XRobot 版本）
xrobot.lock              模块的精确 commit
User/main.cpp            入口：注册硬件并调用 XROBOT_MAIN()
User/bsp_common.hpp      平台初始化、终端、文件日志
User/xrobot.yaml         默认产品配置
User/RunConfig/*.yaml    其他产品配置（hik、sentry、vision_capture）
libxr/                   LibXR submodule
```

`User/xrobot_main.hpp` 和 `Modules/CMakeLists.txt` 由 `xrobot` 生成，不提交。

## 准备

```bash
git submodule update --init --recursive
pip install xrobot==1.0.0      # 与 Modules/modules.yaml 的 xrobot: 一致
xrobot setup                   # 拉取模块、检查所有配置、生成入口头文件
```

OpenVINO 不在 CMake 默认搜索路径时，设置 `OpenVINO_DIR` 或 `CMAKE_PREFIX_PATH`。

## 选择产品并构建

```bash
xrobot gen -c User/RunConfig/hik.yaml      # 切换产品；默认是 User/xrobot.yaml
cmake --preset debug
cmake --build --preset debug --target rm_auto_aim
./build/debug/rm_auto_aim
```

构建前 LibXR 会检查 `User/xrobot_main.hpp` 是否比配置、lock、入口和模块头文件新；
过期时构建失败并提示对应的 `xrobot gen -c <配置>` 命令。

## 产品配置

- `User/xrobot.yaml` / `User/RunConfig/hik.yaml`：实机 Hik 相机，硬件触发和 DevC 回传的 IMU。
  Hik 使用 `2x2` 下采样输出 `720x540`。手眼外参写在
  `ArmorTracker.cfg.extrinsic.camera_to_body`（OpenCV 相机系到本体系 `B`）。
- `User/RunConfig/sentry.yaml`：哨兵配置，使用独立标定。
- `User/RunConfig/vision_capture.yaml`：同步采集和标定数据，只实例化相机、同步、
  SharedTopic 收发和 VisionCapture，输出到 `runs/vision_capture/hik_capture/`。

各配置用到的常量写在配置自己的 `constexprs` 段，生成到 `AutoAimRunConfig` 命名空间。

## 裁判数据话题

`SharedTopic` 只按名字查找收到的话题，不会按类型创建话题；而 `ArmorDetector`、`Aimer`
需要带类型的 `robot_game_ref`。因此 `User/main.cpp` 在 `XROBOT_MAIN()` 之前用
`RefereeTypes::RobotGameRefereePack` 创建这个话题，这是入口唯一直接使用模块类型的地方，
`Modules/modules.yaml` 也因此列出 `QDU-Robomaster/Referee`。

## VS Code

在 Remote-SSH 窗口里打开目标机上的仓库。任务：`XRobot: setup`、`XRobot: select <产品>`、
`Build: <产品> debug`；调试配置按产品列出。
