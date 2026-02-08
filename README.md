# EndFieldCheat

`EndFieldCheat` 是一个基于 C++ 的 Windows DLL 项目，使用 **MinHook** 进行函数 Hook，使用 **Dear ImGui** 构建界面。

## 环境要求

- Windows 10/11 x64
- Visual Studio 2019/2022（含 C++ 工具链）
- CMake 3.14+

## 拉取项目

本项目包含子模块（`imgui`、`minhook`）：

```bash
git clone https://github.com/a0yark/EndFieldCheat.git
cd EndFieldCheat
git submodule update --init --recursive
```

## 构建方式

### 方式一：使用脚本（推荐）

```bat
build.bat
```

默认输出目录：`C:\temp\EndfieldBuild\Release\`

### 方式二：手动 CMake

```bash
cmake -B C:/temp/EndfieldBuild -A x64 -S .
cmake --build C:/temp/EndfieldBuild --config Release
```

## 项目结构

- `DllMain.cpp`：DLL 入口
- `Cheat.cpp`：主要逻辑
- `Hook.cpp` / `Hook.h`：Hook 相关实现
- `GameStructs.h`：游戏结构定义
- `imgui/`：Dear ImGui 子模块
- `minhook/`：MinHook 子模块

## 说明

- 首次构建前请确认子模块已初始化。
- 若 `build.bat` 中本机 CMake 路径不同，请按需修改 `CMAKE_EXE`。
