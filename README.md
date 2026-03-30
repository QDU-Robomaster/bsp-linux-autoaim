# BSP Linux AutoAim

基于 `libxr` / XRobot 的 Linux 自瞄仓库。

## 当前能力

- `ArmorDetector`：YOLOv5 + OpenVINO 主检测链路，可选传统灯条几何细化
- `ArmorTracker`：装甲板观测关联、EKF 状态估计、云台姿态 SLERP 插值、固定安装角 `yaw` 优化
- `Aimer`：装甲板选择、弹道迭代、开火容差判断
- `DemoReplay`：用 `Video/demo.avi` 和 `Video/demo.txt` 离线回放整条链路
- `ReplayMetrics`：输出 detector / tracker / aimer 的量化结果

2026 年 3 月 31 日在仓库自带 demo 上的最近一次本地回放结果：

- `lock_rate=0.852`
- `avg_pred_center_px=25.91`
- `avg_pred_corner_px=36.42`
- `tracker_resets=4`

这些数字是当前代码在本机 replay 的参考值，不是固定指标。

## 目录结构

```text
Modules/        功能模块：ArmorDetector、ArmorTracker、Aimer、HikCamera、DemoReplay
User/           应用入口、XRobot 组装配置、生成的 xrobot_main*.hpp
Video/          本地回放素材（demo.avi、demo.txt）
libxr/          框架与底层组件
build/          CMake 构建输出
AGENTS.md       仓库协作/贡献约定
```

## 依赖

- CMake >= 3.10
- 支持 C++17 的编译器
- OpenCV 4
- OpenVINO Runtime
- Hikrobot MVS SDK
  只在使用 `HikCamera` 实机运行时需要
- `xrobot` 工具链
  修改 `User/xrobot.yaml` 或 `User/xrobot_demo.yaml` 后需要重新生成入口

## 编译

首次拉取建议先初始化子模块：

```bash
git submodule update --init --recursive
cmake -S . -B build
cmake --build build -j$(nproc)
```

可执行文件：

- `./build/rm_auto_aim`：实机模式
- `./build/rm_auto_aim_demo`：离线 demo 回放

如果改了 XRobot YAML 组装文件，先重新生成入口：

```bash
xrobot_gen_main
```

不要手改 `User/xrobot_main.hpp` 或 `User/xrobot_main_demo.hpp`。

## 运行

### 1. 离线回放

```bash
./build/rm_auto_aim_demo
```

默认读取 [User/xrobot_demo.yaml](/home/leo/Documents/bsp-linux-autoaim/User/xrobot_demo.yaml) 中配置的：

- `Video/demo.avi`
- `Video/demo.txt`

适合做算法验证、调参数和看调试界面。

### 2. 实机运行

```bash
./build/rm_auto_aim
```

默认读取 [User/xrobot.yaml](/home/leo/Documents/bsp-linux-autoaim/User/xrobot.yaml)，使用 `HikCamera` 采图。需要相机、MVS SDK 和对应外参配置正确。

## 调试与配置

- 检测调试窗口：将 `armor_detector.debug.preview` 设为 `true`
- 跟踪调试窗口：将 `armor_tracker.debug.preview` 设为 `true`
- 回放模式下建议优先打开 tracker 预览，它会显示候选装甲板、预测装甲板矩形、EKF 状态、`jumped`、NIS 和投影误差
- `User/xrobot_demo.yaml` 用于离线验证
- `User/xrobot.yaml` 用于实机运行

关键配置项：

- 相机内参、畸变、分辨率：`HikCamera.info` 或 `DemoReplay.info`
- 相机到云台外参：`armor_tracker.cfg.frames`
- 检测参数：`armor_detector.cfg.yolo` / `traditional`
- 瞄准参数：`aimer.cfg`

## 说明

- 根目录的 `tracker.sh` 是历史脚本，不是当前 CMake 程序入口
- 模块级说明可分别查看 `Modules/*/README.md`
- 贡献/协作约定见 [AGENTS.md](/home/leo/Documents/bsp-linux-autoaim/AGENTS.md)
