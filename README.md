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

## Generate

默认配置：

```bash
python3 -m xrobot.GenerateMain --output User/xrobot_main.hpp
```

指定运行配置：

```bash
python3 -m xrobot.GenerateMain --config User/RunConfig/hik.yaml --output User/xrobot_main.hpp
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
