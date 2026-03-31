# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build & Run

Initialize submodules before the first build:
```sh
git submodule update --init --recursive
```

Configure and build:
```sh
cmake -S . -B build -G Ninja
cmake --build build
```

Run the application (HikCamera builds require MVS SDK; the script sanitizes `LD_LIBRARY_PATH` for `/opt/MVS/lib/64`):
```sh
./run_demo.sh
# or directly:
./build/rm_auto_aim
```

Build with debug/preview tools enabled:
```sh
cmake -S . -B build -G Ninja -DAUTO_AIM_ENABLE_DEBUG_TOOLS=ON -DAUTO_AIM_PREVIEW_IMAGE=ON
cmake --build build
```

Build and run libxr unit tests:
```sh
cmake -S libxr -B build/libxr-test -DLIBXR_TEST_BUILD=True
cmake --build build/libxr-test
./build/libxr-test/test
```

Generate the `table.bin` lookup table required by ArmorTracker (only needed when `SolveTrajectory::TABLE` mode is used):
```sh
# Uncomment main() in Modules/ArmorTracker/TableGenerator.cpp, then:
cd Modules/ArmorTracker
g++ -o TableGenerator TableGenerator.cpp -O3
./TableGenerator
mv table.bin ../../build/table.bin
# Re-comment main() afterwards
```

The root build exports `compile_commands.json` (via `CMAKE_EXPORT_COMPILE_COMMANDS ON`) for clangd.

## XRobot workflow

### Hard rule

`User/xrobot.yaml` is the only source of truth for module composition and constructor/runtime parameters.

Any module add/remove/reconfigure flow must follow this order:
1. modify `User/xrobot.yaml` directly or via `xrobot_*` tools
2. regenerate `User/xrobot_main.hpp` with `xrobot_gen_main` (or `xrobot_setup` if it triggers regeneration in the current environment)
3. build with CMake

Do **not** hand-edit `User/xrobot_main.hpp` and then compile. If `xrobot.yaml` and `xrobot_main.hpp` disagree, treat `xrobot_main.hpp` as stale generated code and regenerate it.

### Common xrobot commands

These commands are inferred from module READMEs and CI workflows in this repo:

```sh
# one-time environment setup
xrobot_src_man add-source https://qdu-robomaster.github.io/qdu-future-modules/index.yaml
xrobot_src_man add-source https://xrobot-org.github.io/xrobot-modules/index.yaml
xrobot_setup

# initialize XRobot project files/User directory in a fresh repo
xrobot_init_mod

# create a new module skeleton
xrobot_creat_mod <ModuleName>

# add a module instance into User/xrobot.yaml
xrobot_add_mod <ModuleName> [--instance-id <instance_id>]

# regenerate User/xrobot_main.hpp from User/xrobot.yaml
xrobot_gen_main
```

Observed behavior from this repo:
- `xrobot_add_mod` appends module entries into `User/xrobot.yaml`; module READMEs use it to add instances such as `HikCamera`.
- `xrobot_gen_main` regenerates the auto-instantiation header included by [User/main.cpp](User/main.cpp#L16).
- CI also runs `xrobot_setup` after editing module composition; in this environment it appears to refresh generated XRobot artifacts, so if `xrobot_gen_main` is unavailable or wrapped by setup, regenerate before building.
- `xrobot_src_man add-source ...` registers remote module indexes used by XRobot tooling.
- `xrobot_init_mod` is used in CI to initialize a minimal project before adding modules.

### Typical module edit flow

```sh
# add/create modules as needed
xrobot_creat_mod NewModule
xrobot_add_mod HikCamera --instance-id hik_camera
xrobot_add_mod ArmorDetector
xrobot_add_mod ArmorTracker

# then review/edit User/xrobot.yaml manually
# then regenerate generated code
xrobot_gen_main

# then build
cmake -S . -B build -G Ninja
cmake --build build
```

For this repository specifically, parameter tuning belongs in `User/xrobot.yaml` (camera intrinsics, detector thresholds, tracker EKF noise, ballistic solver offsets, planner thresholds). After any YAML edit, regenerate main before compiling.

## Architecture

This is a RoboMaster auto-aim pipeline running on Linux. The build target is `rm_auto_aim`.

### Data flow
```
Camera (HikCamera / VideoFileCamera)
  └─[topic: image frame + CameraInfo]─>
ArmorDetector  (YoloV5 + traditional light-bar detector)
  └─[topic: detected armors]─>
ArmorTracker   (EKF + trajectory solver + fire planner)
  └─[topic: aim command]
```

All inter-module communication uses **LibXR topics** (pub/sub), not direct function calls. Modules publish and subscribe during construction; the main loop only calls `appmgr.MonitorAll()`.

### Key directories

- `User/` — application entry point. `main.cpp` sets up the terminal thread, log callback (debug mode), and `HardwareContainer`, then calls `XRobotMain`. `xrobot_main.hpp` contains the auto-generated module instantiations. `xrobot.yaml` holds all constructor and runtime parameters in a structured format.
- `Modules/` — each subdirectory is a self-contained module with its own `CMakeLists.txt`, auto-included by `Modules/CMakeLists.txt`. Modules register themselves with `ApplicationManager` and interact only via LibXR topics.
- `libxr/` — framework submodule providing threads, topics, terminal (REPL over stdin), RamFS, logging, and Linux/platform drivers. Tests live in `libxr/test/`.
- `Tools/TrackerPlotter/` — standalone OpenCV debug visualizer; only compiled when `AUTO_AIM_ENABLE_DEBUG_TOOLS=ON`.

### Runtime composition in this repo

The current checked-in pipeline is defined in `User/xrobot.yaml` and materialized in `User/xrobot_main.hpp`:
- `VideoFileCamera` or `HikCamera` provides frames and camera calibration
- `ArmorDetector` performs traditional light-bar detection plus optional YoloV5 refinement
- `ArmorTracker` performs target association, EKF state estimation, ballistic solving, and fire decision planning

`User/main.cpp` is intentionally thin: it initializes LibXR platform services, starts the terminal thread, optionally registers the log topic callback in debug builds, and then transfers control to `XRobotMain`.

### CMake options

| Option | Default | Effect |
|---|---|---|
| `AUTO_AIM_ENABLE_DEBUG_TOOLS` | OFF | Enables recorder, overlay, plotter, verbose logging (level 4), and `tracker_plotter` target |
| `AUTO_AIM_PREVIEW_IMAGE` | OFF | Enables OpenCV preview windows (forced OFF if debug tools are OFF) |
| `AUTO_AIM_ENABLE_HIK_CAMERA` | OFF | Builds HikCamera module (requires Hikrobot MVS SDK at `/opt/MVS`) |

### Dependencies

- OpenCV 4 (`core`, `imgproc`, `highgui`)
- `libgpiod-dev` (Linux GPIO, pulled in by libxr)
- Hikrobot MVS SDK at `/opt/MVS` (only for `HikCamera`)
- Optional: `libwpa-client-dev`, `libnm-dev` (Wi-Fi features in libxr)

## Coding Style

C++17. Formatting: Google-based `.clang-format` with 2-space indentation, Allman braces, 90-column limit. `PascalCase` for classes, methods, and free functions; `UPPER_CASE` for constants. Compiler flags include `-Wall -Werror -g -O0`.

## Commits

Short single-line subjects, often in Chinese (e.g. `修复错误的模块名`). Keep messages concise and scoped to one change.
