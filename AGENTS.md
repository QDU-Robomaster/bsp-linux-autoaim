# Repository Guidelines

## Project Structure & Module Organization
`User/` contains the application entry point (`main.cpp`), runtime composition in `xrobot.yaml`, and the generated `xrobot_main.hpp`. `Modules/` holds feature modules such as `ArmorDetector`, `ArmorTracker`, and `HikCamera`; each module keeps its own `CMakeLists.txt`, headers, sources, and any model files. `libxr/` is the framework submodule; its unit tests live in `libxr/test/`. Treat `build/` and `Video/` as generated/runtime output, not source.

## Build, Test, and Development Commands
Run `git submodule update --init --recursive` before the first build. Install XRobot tooling with `pip install xrobot`, then run `xrobot_setup` when bootstrapping the repo or refreshing generated XRobot files.

Use `cmake -S . -B build -G Ninja` to configure the main `rm_auto_aim` target and export `compile_commands.json`, then `cmake --build build` to compile it. For framework tests, run `cmake -S libxr -B build/libxr-test -DLIBXR_TEST_BUILD=True`, `cmake --build build/libxr-test`, and `./build/libxr-test/test`.

## Coding Style & Naming Conventions
This repo uses C++17 with `-Wall -Werror`. Follow `.clang-format`: Google base, 2-space indentation, Allman braces, and a 90-column limit. Use `PascalCase` for classes, modules, and functions, `UPPER_CASE` for constants/macros, and keep existing file naming patterns such as `ArmorTracker.cpp` and `extended_kalman_filter.cpp`.

`User/xrobot.yaml` is the source of truth for module composition and parameters. Do not hand-edit `User/xrobot_main.hpp`; regenerate it with `xrobot_gen_main` after YAML changes.

## Testing Guidelines
Root CI currently verifies configure + build, while checked-in unit tests live under `libxr/test/`. At minimum, run a local CMake build for every change. Run the `libxr` test binary when touching framework code, shared utilities, or threading/topic behavior. No coverage threshold is defined; prefer targeted regression checks for detector/tracker changes.

## Commit & Pull Request Guidelines
Recent history uses short, single-line, scoped commit subjects, often in Chinese, for example `修复错误的模块名` and `更新libxr版本与终端线程堆栈大小`. Keep commits focused and avoid mixing generated files with unrelated refactors.

Pull requests should summarize the affected module or pipeline stage, list the build/test commands you ran, and link the relevant issue or task. Include screenshots or short video evidence only when preview/debug output or vision behavior changed.
